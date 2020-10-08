//
// Author: Shareef Abdoul-Raheem
// File:   lexer.rs
//

pub struct Lexer {
  source: String,
  cursor: usize,
  pub line_no: usize,
}

#[derive(PartialEq, Debug)]
pub struct TokenText {
  pub line_no_start: usize,
  pub line_no_end_with_content: usize,
  pub line_no_end: usize,
  pub text: String,
}

#[derive(PartialEq, Debug)]
pub enum Token {
  Tag(String),
  Text(TokenText),
  Character(char),
  Error(String),
  EndOfFile,
}

trait CharExt {
  fn is_special_character(&self) -> bool;
}

impl CharExt for char {
  fn is_special_character(&self) -> bool {
    return *self == '@' || *self == '{' || *self == '}' || *self == '(' || *self == ')';
  }
}

impl Lexer {
  pub fn new(src: String) -> Lexer {
    Lexer {
      source: src,
      cursor: 0,
      line_no: 1,
    }
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
        _ => {
          if c.is_special_character() {
            self.advance_cursor();
            return Token::Character(c);
          } else {
            return self.parse_text_block();
          }
        }
      }
    }

    return Token::EndOfFile;
  }

  fn parse_tag_name(&mut self) -> Token {
    self.advance_cursor(); // Skip over '@'

    // Tag names can be represented by quotes to have spaces in them.
    if self.current_char() == '\"' {
      self.advance_cursor(); // Skip over '"'

      let name_start = self.cursor;
      let mut name_length = 0;

      while self.current_char() != '\"' {
        self.advance_cursor();

        if !self.is_not_at_end() {
          return Token::Error("Unterminated Tag name string".to_string());
        }
        name_length += 1;
      }

      self.advance_cursor(); // Skip over '"'

      return Token::Tag(self.source[name_start..(name_start + name_length)].to_string());
    } else {
      let name_start = self.cursor;
      let mut name_length = 0;

      while self.current_char().is_ascii_alphanumeric()
        || self.current_char().is_ascii_digit()
        || self.current_char() == '_'
      {
        self.advance_cursor();

        if !self.is_not_at_end() {
          return Token::Error("Unterminated Tag name string".to_string());
        }
        name_length += 1;
      }

      return Token::Tag(self.source[name_start..(name_start + name_length)].to_string());
    }
  }

  fn parse_text_block(&mut self) -> Token {
    let mut text_block = String::new();
    let line_no_start = self.line_no;
    let mut line_no_with_content = line_no_start;

    while !self.current_char().is_special_character() {
      if !self.is_not_at_end() {
        return Token::Error("Unterminated Text Block".to_string());
      }

      let c = self.current_char();
      let c_was_newline = self.advance_cursor();

      if c == '\\' {
        let escaped_character = self.current_char();
        self.advance_cursor();

        // NOTE(SR): Anything commented out works in C but not Rust but left in for completeness.

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
      text: text_block,
    });
  }

  fn skip_whitespace(&mut self) {
    while self.current_char().is_ascii_whitespace() && self.is_not_at_end() {
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

  fn is_not_at_end(&self) -> bool {
    return self.cursor < self.source.len();
  }
}
