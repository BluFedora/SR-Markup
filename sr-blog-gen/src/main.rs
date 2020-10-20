//
// Author: Shareef Abdoul-Raheem
// File:   main.rs

// Test Run
//   `cargo run -- -l bf.PlatformDLL.dll`

// Notes On Rust:
//      To update rustup : `rustup update`
//   To uninstall rustup : `rustup self uninstall`
//       Open Local Docs : `rustup doc`

pub mod sr;

use sr::ast_processor::*;
use sr::ast::*;
use sr::lexer::*;
use sr::c_api::*;

use structopt::StructOpt; // [https://docs.rs/structopt/0.3.20/structopt/]
extern crate libloading; // [https://docs.rs/libloading/0.5.0/libloading/] [https://doc.rust-lang.org/nomicon/ffi.html]
use libc::{c_char, c_int, c_void, size_t};

use std::path::PathBuf;

fn test_blog_string() -> String {
  String::from_utf8_lossy(include_bytes!("../test-blog.blog")).to_string()
}

// CLI

#[derive(Debug, StructOpt)]
struct Options {
  #[structopt(short, long)]
  pub quiet: bool,

  #[structopt(short = "l", long = "library", parse(from_os_str))]
  pub dll_processor: PathBuf,
}

// Dynamic Libraries

struct DynamicLibProcessor {
  api: BlogAPI,
}

#[allow(non_camel_case_types)]
type bfPlatformAllocator =
  fn(ptr: *mut c_void, old_size: size_t, new_size: size_t, user_data: *mut c_void) -> *mut c_void;

#[repr(C)]
#[allow(non_camel_case_types)]
struct bfPlatformInitParams {
  argc: c_int,
  argv: *mut *mut c_char,
  allocator: *const bfPlatformAllocator,
  user_data: *mut c_void,
}

fn main() {
  println!();

  let cwd = std::env::current_dir();
  let args = std::env::args();

  println!("Running in {:?}", cwd.unwrap());

  for arg in args.enumerate() {
    println!("Arg({}) = {}", arg.0, arg.1);
  }

  let options = Options::from_args();

  println!();

  println!("----------------------------------");
  println!("----- SR Blog Post Generator -----");
  println!("----------------------------------\n");

  println!("Options: {:?}", options);

  let dll_lib = libloading::Library::new(options.dll_processor);

  match dll_lib {
    Ok(library) => unsafe {
      #[allow(non_snake_case)]
      let bfPlatformInit: libloading::Symbol<
        unsafe extern "C" fn(params: bfPlatformInitParams) -> i32,
      > = library.get(b"bfPlatformInit").unwrap();

      let params = bfPlatformInitParams {
        argc: 0,
        argv: std::ptr::null_mut(),
        allocator: std::ptr::null(),
        user_data: std::ptr::null_mut(),
      };

      bfPlatformInit(params);

      #[allow(non_snake_case)]
      let bfPlatformCreateWindow: libloading::Symbol<
        unsafe extern "C" fn(*const c_char, c_int, c_int, u32) -> *mut c_void,
      > = library.get(b"bfPlatformCreateWindow").unwrap();

      let window_name = std::ffi::CString::new("Hello, world!");

      let window =
        bfPlatformCreateWindow(window_name.unwrap().as_ptr(), 1280, 720, 1 << 1 | 1 << 2);

      #[allow(non_snake_case)]
      let bfPlatformDoMainLoop: libloading::Symbol<
        unsafe extern "C" fn(*mut c_void) -> c_void,
      > = library.get(b"bfPlatformDoMainLoop").unwrap();

      bfPlatformDoMainLoop(window);
    },
    Err(msg) => {
      panic!(format!("Failed to load library \"{}\".", msg));
    }
  }

  println!("\n----------------------------------\n");

  let lexer = Lexer::new(test_blog_string());
  let mut parser = Parser::new(lexer);
  let syntax_tree = parser.parse();
  let mut process_blog = DebugProcessor { current_indent: 0 };

  match syntax_tree {
    Some(raw_tree) => raw_tree.visit(&mut process_blog),
    None => {
      eprintln!("ERRORS:");

      for err in &parser.error_log {
        eprintln!("{}", err);
      }
    }
  }
}

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

#[test]
fn test_lexer() {
  let mut lexer = Lexer::new(test_blog_string());

  loop {
    let token = lexer.get_next_token();

    println!("Line({}) Token::{:?}", lexer.line_no, token);
    if token == Token::EndOfFile() {
      break;
    }
  }
}
