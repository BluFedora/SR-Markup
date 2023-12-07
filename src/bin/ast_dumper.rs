//
// Author: Shareef Abdoul-Raheem
// File:   main.rs
//

// Test Run
//   `cargo run --bin "sr-markup-ast-dump" -- --input TestInput.txt`

// Notes On Rust:
//      To update rustup : `rustup update`
//   To uninstall rustup : `rustup self uninstall`
//       Open Local Docs : `rustup doc`

use srmarkup;
use srmarkup::ASTNodeLiteral;
use srmarkup::ASTProcessorVisitResult;

use std::fs::File;
use std::io::Read;

use structopt::StructOpt; // [https://docs.rs/structopt/latest/structopt/index.html]

#[derive(Debug, StructOpt)]
struct Options {
    #[structopt(short, long)]
    pub verbose: bool,

    #[structopt(short, long, default_value = "2")]
    pub indent_spaces: u32,

    #[structopt(long)]
    pub input: String,
}

fn main() {
    let options = Options::from_args();

    println!("SR-Markup Dump");
    if options.verbose {
        let args = std::env::args();
        let cwd = std::env::current_dir();

        println!("Current Working Directory = {:?}:", cwd.unwrap());
        for arg in args.enumerate() {
            println!("Arg[{}] = '{}'", arg.0, arg.1);
        }
    }

    let input_file = File::open(&options.input);

    match input_file {
        Ok(mut file) => {
            let mut source = String::new();
            let source_size = file.read_to_string(&mut source);

            match source_size {
                Ok(_) => {
                    let mut parser = srmarkup::Parser::new(source);
                    let parse_result: srmarkup::ParseResult = parser.parse();

                    match parse_result {
                        Ok(root_node) => {
                            let visit_result = srmarkup::visit_ast(
                                &root_node,
                                &mut DebugProcessor::new(options.indent_spaces),
                            );

                            if visit_result == ASTProcessorVisitResult::Halt {
                                println!("Failed to visit all AST nodes.");
                            }
                        }
                        Err(error_log) => {
                            eprintln!("Parse Error:");
                            for err in &error_log.errors {
                                eprintln!("  Line({}): {}\n", err.line_number, err.message);
                            }
                        }
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

struct DebugProcessor {
    current_indent: u32,
    num_spaces_per_indent: u32,
}

impl DebugProcessor {
    pub fn new(num_spaces_per_indent: u32) -> Self {
        DebugProcessor {
            current_indent: 0,
            num_spaces_per_indent,
        }
    }

    fn indent(&mut self) {
        self.current_indent += 1;
    }

    fn print_indent(&self) {
        for _i in 0..self.current_indent {
            for _j in 0..self.num_spaces_per_indent {
                print!(" ");
            }
        }
    }

    fn unindent(&mut self) {
        self.current_indent -= 1;
    }
}

impl srmarkup::IASTProcessor for DebugProcessor {
    fn visit_begin_root(&mut self, _: &srmarkup::ASTNodeRoot) -> ASTProcessorVisitResult {
        println!("(root-begin){{");
        self.indent();
        return ASTProcessorVisitResult::Continue;
    }

    fn visit_begin_tag(&mut self, tag_node: &srmarkup::ASTNodeTag) -> ASTProcessorVisitResult {
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
        return ASTProcessorVisitResult::Continue;
    }

    fn visit_text(&mut self, text_node: &srmarkup::ASTNodeText) -> ASTProcessorVisitResult {
        self.print_indent();
        println!("TEXT({})", text_node.text);
        return ASTProcessorVisitResult::Continue;
    }

    fn visit_literal(&mut self, literal_node: &ASTNodeLiteral) -> ASTProcessorVisitResult {
        self.print_indent();
        println!("LITERAL({:?})", literal_node);
        return ASTProcessorVisitResult::Continue;
    }

    fn visit_end_tag(&mut self, _: &srmarkup::ASTNodeTag) {
        self.unindent();
        self.print_indent();
        println!("}}");
    }

    fn visit_end_root(&mut self, _: &srmarkup::ASTNodeRoot) {
        self.unindent();
        self.print_indent();
        println!("}}(root-end)");
    }
}
