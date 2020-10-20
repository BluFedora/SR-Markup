//
// Author: Shareef Abdoul-Raheem
// File:   ast.rs
//

use crate::sr::ast_processor::IASTProcessor;
use crate::Lexer;
use crate::Token;
use crate::Token::{
  BoolLiteral, Character, EndOfFile, Error, NumberLiteral, StringLiteral, Tag, Text,
};
use crate::TokenTag;
use crate::TokenText;
use std::collections::HashMap;

// AST Nodes

pub type ASTNodePtr = Box<ASTNode>;
pub type ASTNodeList = Vec<ASTNodePtr>;

pub struct ASTNodeRoot {
  pub children: ASTNodeList,
}

pub struct ASTNodeTag {
  pub text: String,
  pub children: ASTNodeList,
  pub attributes: HashMap<String, ASTNodeLiteral>,
}

pub struct ASTNodeText {
  pub text: String,
}

#[derive(Debug)]
pub enum ASTNodeLiteral {
  Str(String),
  Float(f64),
  Bool(bool),
}

pub enum ASTNode {
  Root(ASTNodeRoot),
  Tag(ASTNodeTag),
  Text(ASTNodeText),
  Literal(ASTNodeLiteral),
}

impl ASTNode {
  pub fn visit(&self, processor: &mut dyn IASTProcessor) {
    match self {
      ASTNode::Root(r) => {
        processor.visit_begin_root(r);

        for child in &r.children {
          child.visit(processor);

          if processor.has_error() {
            break;
          }
        }

        processor.visit_end_root(r);
      }
      ASTNode::Tag(t) => {
        processor.visit_begin_tag(t);

        for child in &t.children {
          child.visit(processor);
          if processor.has_error() {
            break;
          }
        }
        processor.visit_end_tag(t);
      }
      ASTNode::Text(t) => processor.visit_text(t),
      ASTNode::Literal(l) => processor.visit_literal(l),
    }
  }
}

fn make_empty_token_text() -> Token {
  return Token::Text(TokenText {
    line_no_start: 0,
    line_no_end_with_content: 0,
    line_no_end: 0,
    text: "".to_string(),
  });
}

// Parser

pub struct Parser {
  lexer: Lexer,
  current_token: Token,
  pub error_log: Vec<String>,
}

impl Parser {
  pub fn new(lex: Lexer) -> Self {
    Parser {
      lexer: lex,
      current_token: Token::EndOfFile(),
      error_log: Vec::new(),
    }
  }

  pub fn parse(&mut self) -> Option<ASTNodePtr> {
    let mut root_node = ASTNodeRoot {
      children: Vec::new(),
    };

    self.advance_token();
    self.parse_impl(&mut root_node.children);

    return if self.error_log.is_empty() {
      Some(Box::new(ASTNode::Root(root_node)))
    } else {
      None
    };
  }

  fn parse_impl(&mut self, parent_child_list: &mut ASTNodeList) {
    loop {
      let current_token = self.current_token.clone();

      match current_token {
        Tag(ref tt) => {
          let tt_node = self.parse_tag_block(&tt);

          if tt_node.is_some() {
            parent_child_list.push(tt_node.unwrap());
          }
        }
        StringLiteral(ref str_lit) => {
          let child_node = Box::new(ASTNode::Literal(ASTNodeLiteral::Str(str_lit.clone())));
          self.advance_token();

          parent_child_list.push(child_node);
        }
        NumberLiteral(number) => {
          let child_node = Box::new(ASTNode::Literal(ASTNodeLiteral::Float(number)));
          self.advance_token();

          parent_child_list.push(child_node);
        }
        BoolLiteral(value) => {
          let child_node = Box::new(ASTNode::Literal(ASTNodeLiteral::Bool(value)));
          self.advance_token();

          parent_child_list.push(child_node);
        }
        Text(ref txt) => {
          let child_node = Box::new(ASTNode::Text(ASTNodeText {
            text: txt.text.clone(),
          }));
          self.advance_token();

          parent_child_list.push(child_node);
        }
        Character(_value) => {
          break;
        }
        Error(err_msg) => {
          self.error_panic(format!("Tokenizer {}", err_msg));
        }
        EndOfFile() => {
          break;
        }
      }
    }
  }

  fn token_to_ast_literal(tok: Token) -> ASTNodeLiteral {
    match tok {
      StringLiteral(ref str_lit) => return ASTNodeLiteral::Str(str_lit.clone()),
      NumberLiteral(number) => return ASTNodeLiteral::Float(number),
      BoolLiteral(value) => return ASTNodeLiteral::Bool(value),
      _ => panic!("The token was not a literal"),
    }
  }

  fn parse_tag_block(&mut self, tag: &TokenTag) -> Option<ASTNodePtr> {
    let mut tag_node = ASTNodeTag::new(tag.text.clone());

    self.advance_token();

    if self.expect(&Token::Character('(')) {
      while !self.expect(&Token::Character(')')) {
        let variable_name = self.current_token.clone();

        self.require(
          &make_empty_token_text(),
          &format!("Variable must be a string name but got {}", variable_name),
        );
        self.require(
          &Token::Character('='),
          &format!("'{}' must be assigned to", variable_name),
        );

        let literal_value = self.current_token.clone();

        if literal_value.is_literal() {
          self.advance_token();

          let var_name_as_str = match variable_name {
            Token::Text(value) => value.text,
            _ => panic!("The variable must be a text node"),
          };

          tag_node
            .attributes
            .insert(var_name_as_str, Parser::token_to_ast_literal(literal_value));
        } else {
          self.error_panic(format!(
            "'{}' must be assigned a literal value",
            variable_name
          ));
        }

        //
        // NOTE(Shareef):
        //   Commas are optional, since all literals
        //   have a defined token there is no ambiguity when
        // 
        self.expect(&Token::Character(','));
      }
    }

    // NOTE(Shareef):
    //   Tag Body is optional
    // if self.require(&Token::Character('{')) {
    if self.expect(&Token::Character('{')) {
      while !self.expect(&Token::Character('}')) {
        self.parse_impl(&mut tag_node.children);
      }
    }

    return Some(Box::new(ASTNode::Tag(tag_node)));
  }

  fn current_token_is(&self, token: &Token) -> bool {
    let current_type = std::mem::discriminant(&self.current_token);
    let token_type = std::mem::discriminant(token);

    if current_type == token_type {
      if token_type != std::mem::discriminant(&Token::Character('_'))
        || self.current_token == *token
      {
        return true;
      }
    }

    return false;
  }

  fn require(&mut self, token: &Token, err_message: &String) -> bool {
    if self.current_token_is(token) {
      self.advance_token();
      return true;
    }

    self.error_panic(format!(
      "Expected {} but got {}, {}",
      *token, self.current_token, err_message
    ));

    // Prevent infinite loops by just returning true when at the end of a file.
    return self.current_token == Token::EndOfFile();
  }

  fn expect(&mut self, token: &Token) -> bool {
    if self.current_token_is(token) {
      self.advance_token();
      return true;
    }

    let is_at_end_of_file = self.current_token == Token::EndOfFile();

    if is_at_end_of_file {
      self.error_panic(format!("Unexpected eof of while searching for {}", *token));
    }
    // Prevent infinite loops by just returning true when at the end of a file.
    return is_at_end_of_file;
  }

  fn advance_token(&mut self) -> &Token {
    self.current_token = self.lexer.get_next_token();
    &self.current_token
  }

  fn error_panic(&mut self, message: String) {
    // Advance the token as not to get stuck in infinite loops.
    self.advance_token();
    self
      .error_log
      .push(format!("Line({}): {}.", self.lexer.line_no, message));
  }
}

impl ASTNodeTag {
  pub fn new(text: String) -> Self {
    Self {
      text: text,
      children: Default::default(),
      attributes: Default::default(),
    }
  }
}
