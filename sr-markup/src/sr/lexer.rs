//
// Author: Shareef Abdoul-Raheem
// File:   lexer.rs
//

// Token

#[derive(PartialEq, Debug, Clone)]
pub struct TokenTag {
  pub text: String,
}

#[derive(PartialEq, Debug, Clone)]
pub struct TokenText {
  pub line_no_start: usize,
  pub line_no_end_with_content: usize,
  pub line_no_end: usize,
  pub text: String,
}

#[derive(PartialEq, Debug, Clone)]
pub enum Token {
  Tag(TokenTag),
  StringLiteral(String),
  NumberLiteral(f64),
  BoolLiteral(bool),
  Text(TokenText),
  Character(char),
  Error(String),
  EndOfFile(),
}

impl Token {
  pub fn is_literal(&self) -> bool {
    match self {
      Token::StringLiteral(_value) => return true,
      Token::NumberLiteral(_value) => return true,
      Token::BoolLiteral(_value) => return true,
      _ => return false,
    }
  }
}

impl std::fmt::Display for Token {
  fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
    write!(f, "{:?}", self)
  }
}

// Lexer

#[derive(Copy, Clone, PartialEq)]
pub enum LexerMode {
  Text, // This mode is loose and will allow spaces in identifier type of characters groups.
  Code, // This mode is strict and will only allow identifiers typical of programming languages.
}

pub struct Lexer {
  source: String,
  cursor: usize,
  pub line_no: usize,
  mode: LexerMode,
  mode_stack: Vec<LexerMode>,
}

impl Lexer {
  pub fn new(src: String) -> Self {
    Lexer {
      source: src,
      cursor: 0,
      line_no: 1,
      mode: LexerMode::Text,
      mode_stack: Default::default(),
    }
  }

  pub fn push_mode(&mut self, mode: LexerMode) {
    self.mode_stack.push(self.mode);
    self.mode = mode;
  }

  pub fn pop_mode(&mut self) {
    self.mode = self.mode_stack.pop().unwrap();
  }

  pub fn get_next_token(&mut self) -> Token {
    while self.is_not_at_end() {
      if self.current_char().is_ascii_whitespace() {
        self.skip_whitespace();
        continue;
      }

      let c = self.current_char();

      match c {
        '@' => return self.parse_tag_name(),
        '\"' => {
          return match self.parse_quoted_string() {
            Ok(quoted_string) => Token::StringLiteral(quoted_string),
            Err(err_token) => err_token,
          }
        }
        '0'..='9' => return self.parse_numeric_literal(),
        _ => {
          let src_len_left = self.source.len() - self.cursor;

          if c.is_special_character() || (c == ',' && self.mode == LexerMode::Code) {
            self.advance_cursor(); // ','
            return Token::Character(c);
          } else if src_len_left >= 4 && self.source[self.cursor..(self.cursor + 4)] == *"true" {
            self.advance_cursor(); // 't'
            self.advance_cursor(); // 'r'
            self.advance_cursor(); // 'u'
            self.advance_cursor(); // 'e'

            return Token::BoolLiteral(true);
          } else if src_len_left >= 5 && self.source[self.cursor..(self.cursor + 4)] == *"false" {
            self.advance_cursor(); // 'f'
            self.advance_cursor(); // 'a'
            self.advance_cursor(); // 'l'
            self.advance_cursor(); // 's'
            self.advance_cursor(); // 'e'
            return Token::BoolLiteral(false);
          } else {
            return self.parse_text_block();
          }
        }
      }
    }

    return Token::EndOfFile();
  }

  fn parse_numeric_literal(&mut self) -> Token {
    let number_start = self.cursor;

    while self.current_char().is_ascii_digit() || self.current_char() == '.' {
      self.advance_cursor();

      if self.is_at_end() {
        break;
      }
    }

    let number_end = self.cursor;
    let number = self.source[number_start..number_end].parse::<f64>();

    match number {
      Ok(value) => return Token::NumberLiteral(value),
      Err(e) => return Token::Error(e.to_string()),
    }
  }

  fn parse_quoted_string(&mut self) -> Result<String, Token> {
    self.advance_cursor(); // Skip over '"'

    let name_start = self.cursor;
    let mut name_length = 0;

    while self.current_char() != '\"' {
      self.advance_cursor();

      if self.is_at_end() {
        return Err(Token::Error("Unterminated Tag name string".to_string()));
      }

      name_length += 1;
    }

    self.advance_cursor(); // Skip over '"'

    return Ok(self.source[name_start..(name_start + name_length)].to_string());
  }

  fn parse_tag_name(&mut self) -> Token {
    self.advance_cursor(); // Skip over '@'

    // Tag names can be represented by quotes to allow for spaces in the identifier.
    if self.current_char() == '\"' {
      return match self.parse_quoted_string() {
        Ok(token_str) => Token::Tag(TokenTag { text: token_str }),
        Err(err_token) => err_token,
      };
    } else {
      let name_start = self.cursor;
      let mut name_length = 0;

      while self.current_char().is_ascii_alphanumeric()
        || self.current_char().is_ascii_digit()
        || self.current_char() == '_'
      {
        self.advance_cursor();

        if self.is_at_end() {
          return Token::Error("Unterminated Tag name string".to_string());
        }
        name_length += 1;
      }

      return Token::Tag(TokenTag {
        text: self.source[name_start..(name_start + name_length)].to_string(),
      });
    }
  }

  fn parse_text_block(&mut self) -> Token {
    let mut text_block = String::new();
    let line_no_start = self.line_no;
    let mut line_no_with_content = line_no_start;

    while self.is_not_at_end()
      && !self.current_char().is_text_block_ending_character(self.mode)
      && self.current_char() != '\"'
    {
      if self.is_at_end() {
        return Token::Error("Unterminated Text Block".to_string());
      }

      let c = self.current_char();
      let c_was_newline = self.advance_cursor();

      if c == '\\' {
        let escaped_character = self.current_char();
        self.advance_cursor();

        // NOTE(Shareef):
        //   Anything commented out works in C but not Rust but 
        //   left in for completeness.

        let cc = match escaped_character {
          // 'a' => '\a',
          // 'b' => '\b',
          // 'f' => '\f',
          'n' => '\n',
          'r' => '\r',
          't' => '\t',
          // 'v' => '\v',
          '\\' => '\\',
          '\'' => '\'',
          '\"' => '\"',
          // '\?' => '\?',
          _ => escaped_character,
        };

        text_block.push(cc);
      } else if c_was_newline {
        self.skip_whitespace();
        text_block.push(' ');
      } else {
        text_block.push(c);
        line_no_with_content = self.line_no;
      }
    }

    return Token::Text(TokenText {
      line_no_start: line_no_start,
      line_no_end_with_content: line_no_with_content,
      line_no_end: self.line_no,
      text: text_block.trim_end().to_string(),
    });
  }

  fn skip_whitespace(&mut self) {
    while self.is_not_at_end() && self.current_char().is_ascii_whitespace() {
      self.advance_cursor();
    }
  }

  fn advance_cursor(&mut self) -> bool {
    let is_win_newline = self.current_char() == '\r';
    let is_newline = self.current_char() == '\n';

    self.cursor += 1;

    if is_win_newline || is_newline {
      if is_win_newline && self.current_char() == '\n' {
        self.cursor += 1;
      }

      self.line_no += 1;
    }

    return is_win_newline || is_newline;
  }

  fn current_char(&self) -> char {
    return self.char_at(self.cursor);
  }

  fn char_at(&self, index: usize) -> char {
    return self.source.chars().nth(index).unwrap();
  }

  fn is_at_end(&self) -> bool {
    return !self.is_not_at_end();
  }

  fn is_not_at_end(&self) -> bool {
    return self.cursor < self.source.len();
  }
}

// Character Helpers

trait CharExt {
  fn is_special_character(&self) -> bool;
  fn is_text_block_ending_character(&self, mode: LexerMode) -> bool;
}

impl CharExt for char {
  fn is_special_character(&self) -> bool {
    return *self == '@'
      || *self == '{'
      || *self == '}'
      || *self == '('
      || *self == ')'
      || *self == '=';
  }

  fn is_text_block_ending_character(&self, mode: LexerMode) -> bool {
    match mode {
      LexerMode::Text => return *self == '@'
      || *self == '{'
      || *self == '}'
      || *self == '=',
      LexerMode::Code => return *self == '@'
      || *self == '{'
      || *self == '}'
      || *self == '=' ||
      *self == ' ',
    }
  }
}
