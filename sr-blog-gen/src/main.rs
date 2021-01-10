//
// Author: Shareef Abdoul-Raheem
// File:   main.rs
//

// Test Run
//   `cargo run -- -i test-blog.blog -l ../sr.GenBlog.dll`
//   `cargo run -- -i test-blog.blog -l sr.GenBlog.dll > TEST.txt`

// Notes On Rust:
//      To update rustup : `rustup update`
//   To uninstall rustup : `rustup self uninstall`
//       Open Local Docs : `rustup doc`

pub mod sr;

use sr::ast::*;
use sr::ast_processor::*;
use sr::c_api::*;
use sr::lexer::*;

use structopt::StructOpt; // [https://docs.rs/structopt/0.3.20/structopt/]
extern crate libloading; // [https://docs.rs/libloading/0.5.0/libloading/] [https://doc.rust-lang.org/nomicon/ffi.html]
use libc::{c_char, c_void};

use std::fs::File;
use std::io::Read;
use std::path::PathBuf;

// CLI

#[derive(Debug, StructOpt)]
struct Options {
  #[structopt(short, long)]
  pub verbose: bool,

  #[structopt(short, long)]
  pub input: String,

  #[structopt(short = "l", long = "library", parse(from_os_str), default_value = "")]
  pub dll_processor: PathBuf,
}

fn main() {
  let options = Options::from_args();

  if options.verbose {
    let cwd = std::env::current_dir();
    let args = std::env::args();

    println!("\nCWD {:?}:", cwd.unwrap());
    for arg in args.enumerate() {
      println!("  Arg({}) = '{}'", arg.0, arg.1);
    }

    println!("----------------------------------");
    println!("----- SR Blog Post Generator -----");
    println!("----------------------------------\n");
  }

  let input_file = File::open(&options.input);

  match input_file {
    Ok(mut file) => {
      let mut source = String::new();
      let source_size = file.read_to_string(&mut source);

      match source_size {
        Ok(_) => {
          if options.dll_processor.as_os_str().is_empty() {
            process_file(&mut DebugProcessor { current_indent: 0 }, source);
          } else {
            let mut processor = DynamicLibProcessor::new(options.dll_processor);
            if processor.err.is_some() {
              eprintln!(
                "[ERROR] Failed to load dynamic lib, {}.",
                processor.err.unwrap()
              );
              return;
            }
            process_file(&mut processor, source);
          }
        }
        Err(msg) => {
          eprintln!(
            "[ERROR] Failed to read file ('{}'), {}.",
            options.input, msg
          );
        }
      }
    }
    Err(msg) => {
      eprintln!(
        "[ERROR] Failed to load file ('{}'), {}.",
        options.input, msg
      );
    }
  }
}

fn process_file(processor: &mut dyn IASTProcessor, source: String) {
  let lexer = Lexer::new(source);
  let mut parser = Parser::new(lexer);
  let syntax_tree = parser.parse();

  match syntax_tree {
    Some(raw_tree) => raw_tree.visit(processor),
    None => {
      eprintln!("ERRORS:");

      for err in &parser.error_log {
        eprintln!("{}", err);
      }
    }
  }
}

// Built-In IASTProcessor(s)

/* Dyn-Lib Processor */

type DynLibInitCallbackFn = unsafe extern "C" fn() -> *mut c_void;
type DynLibProcessCallbackFn =
  unsafe extern "C" fn(node: *const ASTNodeView, user_data: *mut c_void) -> bool;
type DynLibShutdownCallbackFn = unsafe extern "C" fn(user_data: *mut c_void);

struct DynamicLibProcessor {
  node_view_list_stack: Vec<Vec<ASTNodeView>>,
  node_attribute_storage: Vec<Vec<TagAttributeView>>,
  node_list_idx_stack: Vec<usize>,
  library: Option<libloading::Library>,
  err: Option<std::io::Error>,
}

impl DynamicLibProcessor {
  fn new(dll_path: PathBuf) -> Self {
    let dll_lib = libloading::Library::new(dll_path);

    match dll_lib {
      Ok(library) => {
        return Self {
          node_view_list_stack: Default::default(),
          node_attribute_storage: Default::default(),
          node_list_idx_stack: Default::default(),
          library: Some(library),
          err: None,
        };
      }
      Err(msg) => {
        return Self {
          node_view_list_stack: Default::default(),
          node_attribute_storage: Default::default(),
          node_list_idx_stack: Default::default(),
          library: None,
          err: Some(msg),
        };
      }
    }
  }

  fn push_list(&mut self) {
    self.node_list_idx_stack.push(self.node_view_list_stack.len());
    self.node_view_list_stack.push(Default::default());
  }

  fn add_to_list(&mut self, node_data: ASTNodeView) {
    self.current_list_mut().push(node_data);
  }

  fn pop_list(&mut self) {
    self.node_list_idx_stack.pop();
  }

  fn current_list_mut(&mut self) -> &mut Vec<ASTNodeView> {
    let index = self.current_list_index();
    &mut self.node_view_list_stack[index]
  }

  fn current_list(&self) -> &Vec<ASTNodeView> {
    &self.node_view_list_stack[self.current_list_index()]
  }

  fn string_to_view(string_value: &String) -> StringView {
    let str_start = string_value.as_ptr() as *const c_char;

    StringView {
      str_start: str_start,
      str_end: unsafe { str_start.offset(string_value.len() as isize) },
    }
  }

  fn literal_to_view(literal_node: &sr::ast::ASTNodeLiteral) -> ASTNodeLiteralValue {
    return match literal_node {
      ASTNodeLiteral::Str(s) => ASTNodeLiteralValue::AsStr(DynamicLibProcessor::string_to_view(&s)),
      ASTNodeLiteral::Float(f) => ASTNodeLiteralValue::AsNumber(*f),
      ASTNodeLiteral::Bool(b) => ASTNodeLiteralValue::AsBoolean(*b),
    };
  }

  fn list_to_view(list: &Vec<ASTNodeView>) -> ASTNodeListView {
    ASTNodeListView {
      num_nodes: list.len() as u32,
      nodes: list.as_ptr(),
    }
  }

  fn add_attributes(&mut self, attribs: &std::collections::HashMap<String, ASTNodeLiteral>) {
    let index = self.node_attribute_storage.len();
    self.node_attribute_storage.push(Default::default());
    let attrib_list = &mut self.node_attribute_storage[index];

    for attrib in attribs {
      attrib_list.push(TagAttributeView {
        key: DynamicLibProcessor::string_to_view(attrib.0),
        value: DynamicLibProcessor::literal_to_view(attrib.1),
      });
    }
  }

  fn current_attributes(&self) -> &Vec<TagAttributeView> {
    &self.node_attribute_storage[self.node_attribute_storage.len() - 1]
  }

  fn current_list_index(&self) -> usize {
    self.node_list_idx_stack[self.node_list_idx_stack.len() - 1]
  }
}

impl IASTProcessor for DynamicLibProcessor {
  fn has_error(&mut self) -> bool {
    false
  }

  fn visit_begin_root(&mut self, _: &sr::ast::ASTNodeRoot) {
    self.push_list();
  }

  fn visit_begin_tag(&mut self, _: &sr::ast::ASTNodeTag) {
    self.push_list();
  }

  fn visit_text(&mut self, text_node: &sr::ast::ASTNodeText) {
    self.add_to_list(ASTNodeView::TextNode(DynamicLibProcessor::string_to_view(
      &text_node.text,
    )));
  }

  fn visit_literal(&mut self, literal_node: &sr::ast::ASTNodeLiteral) {
    self.add_to_list(ASTNodeView::LiteralNode(
      DynamicLibProcessor::literal_to_view(literal_node),
    ));
  }

  fn visit_end_tag(&mut self, tag_node: &sr::ast::ASTNodeTag) {
    self.add_attributes(&tag_node.attributes);
    let attrib_list = self.current_attributes();
    let attrib_list_len = attrib_list.len() as u32;
    let attrib_list_ptr = attrib_list.as_ptr();
    let current_list = DynamicLibProcessor::list_to_view(&self.current_list());
    self.pop_list();

    self.add_to_list(ASTNodeView::TagNode(
      DynamicLibProcessor::string_to_view(&tag_node.text),
      current_list,
      attrib_list_len,
      attrib_list_ptr,
    ));
  }

  fn visit_end_root(&mut self, _: &sr::ast::ASTNodeRoot) {
    unsafe {
      let mut user_data: *mut c_void = std::ptr::null_mut();
      let library = self.library.as_ref().unwrap();

      // Init
      {
        let init_cb_res: libloading::Result<libloading::Symbol<DynLibInitCallbackFn>> =
          library.get(b"srBlogGenInit");

        match init_cb_res {
          Ok(init_cb) => {
            user_data = init_cb();
          }
          Err(_err) => {
            eprintln!("[WARN]: Could not find 'srBlogGenInit' callback.");
          }
        }
      }

      // Process
      {
        let process_cb_res: libloading::Result<libloading::Symbol<DynLibProcessCallbackFn>> =
          library.get(b"srBlogGenProcess");

        match process_cb_res {
          Ok(process_cb) => {
            let children = self.current_list();

            for node in children {
              process_cb(&*node, user_data);
            }
          }
          Err(_err) => {
            eprintln!("[WARN]: Could not find 'srBlogGenProcess' callback.");
          }
        }
      }

      // Shutdown
      {
        let shutdown_cb_res: libloading::Result<libloading::Symbol<DynLibShutdownCallbackFn>> =
          library.get(b"srBlogGenShutdown");

        match shutdown_cb_res {
          Ok(shutdown_cb) => {
            shutdown_cb(user_data);
          }
          Err(_err) => {
            eprintln!("[WARN]: Could not find 'srBlogGenShutdown' callback.");
          }
        }
      }
    }

    self.pop_list();
  }
}

/* Debug-Processor */

struct DebugProcessor {
  current_indent: u32,
}

impl DebugProcessor {
  fn indent(&mut self) {
    self.current_indent += 1;
  }

  fn print_indent(&self) {
    for _i in 0..self.current_indent {
      print!("  ");
    }
  }

  fn unindent(&mut self) {
    self.current_indent -= 1;
  }
}

impl IASTProcessor for DebugProcessor {
  fn has_error(&mut self) -> bool {
    false
  }

  fn visit_begin_root(&mut self, _: &sr::ast::ASTNodeRoot) {
    println!("ROOT DOC BEGIN:");
    self.indent();
  }

  fn visit_begin_tag(&mut self, tag_node: &sr::ast::ASTNodeTag) {
    self.print_indent();
    println!("Tag({}) {{", tag_node.text);
    self.indent();

    if !tag_node.attributes.is_empty() {
      self.indent();

      self.print_indent();
      println!("Attributes: ");

      self.indent();
      for attrib in &tag_node.attributes {
        self.print_indent();
        println!("'{}' = {:?}", attrib.0, attrib.1);
      }
      self.unindent();

      self.unindent();
    }
  }

  fn visit_text(&mut self, text_node: &sr::ast::ASTNodeText) {
    self.print_indent();
    println!("TEXT({})", text_node.text);
  }

  fn visit_literal(&mut self, literal_node: &sr::ast::ASTNodeLiteral) {
    self.print_indent();
    println!("LITERAL({:?})", literal_node);
  }

  fn visit_end_tag(&mut self, _: &sr::ast::ASTNodeTag) {
    self.unindent();
    self.print_indent();
    println!("}}");
  }

  fn visit_end_root(&mut self, _: &sr::ast::ASTNodeRoot) {
    self.unindent();
    self.print_indent();
    println!("ROOT DOC END:");
  }
}
