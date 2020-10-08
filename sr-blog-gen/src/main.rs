//
// Author: Shareef Abdoul-Raheem
// File:   main.rs
//

// Notes On Rust:
//   To update   rustup: `rustup update`
//   To unistall rustup: `rustup self uninstall`
//      Open Local Docs: `rustup doc`

mod sr;

use sr::lexer::*;

fn main() {
    println!("----------------------------------");
    println!("----- SR Blog Post Generator -----");
    println!("----------------------------------\n");

    let test_blog_file_bytes = include_bytes!("../test-blog.blog");
    let test_blog_file_string = String::from_utf8_lossy(test_blog_file_bytes);
    let mut lexer = Lexer::new(test_blog_file_string.to_string());

    // println!("{}", test_blog_file_string);

    loop {
        let token = lexer.get_next_token();

        println!("Line({}) Token::{:?}", lexer.line_no, token);

        if token == Token::EndOfFile
        {
            break;
        }
    }
}
