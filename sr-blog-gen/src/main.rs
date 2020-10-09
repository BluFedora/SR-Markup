//
// Author: Shareef Abdoul-Raheem
// File:   main.rs
//

// Notes On Rust:
//      To update rustup : `rustup update`
//   To uninstall rustup : `rustup self uninstall`
//       Open Local Docs : `rustup doc`

pub mod sr;

use sr::ast::*;
use sr::lexer::*;

#[test]
fn test_lexer() {
  let test_blog_file_bytes = include_bytes!("../test-blog.blog");
  let test_blog_file_string = String::from_utf8_lossy(test_blog_file_bytes);
  let mut lexer = Lexer::new(test_blog_file_string.to_string());

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

  let test_blog_file_bytes = include_bytes!("../test-blog.blog");
  let test_blog_file_string = String::from_utf8_lossy(test_blog_file_bytes);
  let lexer = Lexer::new(test_blog_file_string.to_string());
  let mut parser = Parser::new(lexer);
  let syntax_tree = parser.parse();

  if syntax_tree.is_some() {
    syntax_tree.unwrap().visit(&mut parser);
  } else {
    println!("ERRORS:");

    for err in &parser.error_log {
      println!("{}", err);
    }
  }
}
