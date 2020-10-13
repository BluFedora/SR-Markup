//
// Author: Shareef Abdoul-Raheem
// File:   main.rs
//

// Notes On Rust:
//      To update rustup : `rustup update`
//   To uninstall rustup : `rustup self uninstall`
//       Open Local Docs : `rustup doc`

pub mod sr;

use crate::sr::ast_processor::IAstProcessor;
use sr::ast::*;
use sr::lexer::*;

fn test_blog_string() -> String {
  String::from_utf8_lossy(include_bytes!("../test-blog.blog")).to_string()
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

fn main() {
  let args = std::env::args();

  println!("----------------------------------");
  println!("----- SR Blog Post Generator -----");
  println!("----------------------------------\n");

  for arg in args.enumerate() {
    println!("Arg({}) = {}", arg.0, arg.1);
  }

  println!("\n----------------------------------\n");

  let lexer = Lexer::new(test_blog_string());
  let mut parser = Parser::new(lexer);
  let syntax_tree = parser.parse();
  let mut process_blog = BlogProcessor { current_indent: 0 };

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

struct BlogProcessor {
  current_indent: u32,
}

impl BlogProcessor {
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

impl IAstProcessor for BlogProcessor {
  fn has_error(&mut self) -> bool {
    false
  }

  fn visit_begin_root(&mut self, _: &sr::ast::AstNodeRoot) {
    println!("ROOT DOC BEGIN:");
    self.indent();
  }

  fn visit_begin_tag(&mut self, tag_node: &sr::ast::AstNodeTag) {
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

  fn visit_text(&mut self, text_node: &sr::ast::AstNodeText) {
    self.print_indent();
    println!("TEXT({})", text_node.text);
  }

  fn visit_literal(&mut self, literal_node: &sr::ast::AstNodeLiteral) {
    self.print_indent();
    println!("LITERAL({:?})", literal_node);
  }

  fn visit_end_tag(&mut self, _: &sr::ast::AstNodeTag) {
    self.unindent();
    self.print_indent();
    println!("}}");
  }

  fn visit_end_root(&mut self, _: &sr::ast::AstNodeRoot) {
    self.unindent();
    self.print_indent();
    println!("ROOT DOC END:");
  }
}
