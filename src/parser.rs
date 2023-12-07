//
// Author: Shareef Abdoul-Raheem
// File:   parser.rs
//

use crate::ast::ASTNode;
use crate::ast::ASTNodeList;
use crate::ast::ASTNodeLiteral;
use crate::ast::ASTNodePtr;
use crate::ast::ASTNodeRoot;
use crate::ast::ASTNodeTag;
use crate::ast::ASTNodeText;

use crate::lexer::LexerMode;
use crate::lexer::Token;
use crate::lexer::TokenTag;
use crate::lexer::TokenText;
use crate::lexer::Lexer;

use std::mem::swap;

pub struct ParseError {
    pub message: String,
    pub line_number: usize,
}

pub struct ParseErrors {
    pub errors: Vec<ParseError>,
}

pub type ParseResult = Result<ASTNodePtr, ParseErrors>;

/// Parses an sr-mark source text provided by the passed in lexer.
pub struct Parser {
    lexer: Lexer,
    current_token: Token,
    error_log: Vec<ParseError>,
}

impl Parser {
    pub fn new(source: String) -> Self {
        Parser {
            lexer: Lexer::new(source),
            current_token: Token::EndOfFile(),
            error_log: Vec::new(),
        }
    }

    pub fn parse(&mut self) -> ParseResult {
        let mut root_node = ASTNodeRoot {
            children: Vec::new(),
        };

        self.advance_token();
        self.parse_impl(&mut root_node.children);

        return if self.error_log.is_empty() {
            Ok(Box::new(ASTNode::Root(root_node)))
        } else {
            let mut errors = ParseErrors { errors: Vec::new() };

            swap(&mut errors.errors, &mut self.error_log);

            Err(errors)
        };
    }

    fn parse_impl(&mut self, parent_child_list: &mut ASTNodeList) {
        loop {
            let current_token = self.current_token.clone();

            match current_token {
                Token::Tag(ref tt) => {
                    let tt_node = self.parse_tag_block(&tt);

                    if tt_node.is_some() {
                        parent_child_list.push(tt_node.unwrap());
                    }
                }
                Token::StringLiteral(ref str_lit) => {
                    let child_node =
                        Box::new(ASTNode::Literal(ASTNodeLiteral::Str(str_lit.clone())));
                    self.advance_token();

                    parent_child_list.push(child_node);
                }
                Token::NumberLiteral(number) => {
                    let child_node = Box::new(ASTNode::Literal(ASTNodeLiteral::Float(number)));
                    self.advance_token();

                    parent_child_list.push(child_node);
                }
                Token::BoolLiteral(value) => {
                    let child_node = Box::new(ASTNode::Literal(ASTNodeLiteral::Bool(value)));
                    self.advance_token();

                    parent_child_list.push(child_node);
                }
                Token::Text(ref txt) => {
                    let child_node = Box::new(ASTNode::Text(ASTNodeText {
                        text: txt.text.clone(),
                    }));
                    self.advance_token();

                    parent_child_list.push(child_node);
                }
                Token::Character(_value) => {
                    //let child_node = Box::new(ASTNode::Text(ASTNodeText {
                    //  text: value.to_string(),
                    //}));
                    // self.advance_token();
                    //parent_child_list.push(child_node);
                    break;
                }
                Token::Error(err_msg) => {
                    self.error_panic(format!("Tokenizer {}", err_msg));
                }
                Token::EndOfFile() => {
                    break;
                }
            }
        }
    }

    fn token_to_ast_literal(tok: Token) -> ASTNodeLiteral {
        match tok {
            Token::StringLiteral(ref str_lit) => return ASTNodeLiteral::Str(str_lit.clone()),
            Token::NumberLiteral(number) => return ASTNodeLiteral::Float(number),
            Token::BoolLiteral(value) => return ASTNodeLiteral::Bool(value),
            _ => panic!("The token was not a literal"),
        }
    }

    fn parse_tag_block(&mut self, tag: &TokenTag) -> Option<ASTNodePtr> {
        let mut tag_node = ASTNodeTag::new(tag.text.clone());

        self.advance_token();

        self.lexer.push_mode(LexerMode::Code);
        if self.expect(&Token::Character('(')) {
            // TODO(SR):
            //   For better error messages I can skip until a ')' as that provides
            //   a pretty good 'sequence point'.

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
                        "'{}' should have been a literal value",
                        variable_name
                    ));
                }

                //
                // NOTE(Shareef):
                //   Commas are optional, since all literals
                //   have a defined token there is no ambiguity.
                //
                self.expect(&Token::Character(','));
            }
        }
        self.lexer.pop_mode();

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

        // Prevent infinite loops by just returning true when at the end of a file.
        return self.current_token == Token::EndOfFile();
    }

    fn advance_token(&mut self) -> &Token {
        self.current_token = self.lexer.get_next_token();
        &self.current_token
    }

    fn error_panic(&mut self, message: String) {
        // Advance the token as not to get stuck in infinite loops and
        // better error messages.
        self.advance_token();
        self.error_log.push(ParseError {
            message: message,
            line_number: self.lexer.line_no,
        });
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
