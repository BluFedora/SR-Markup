//
// Author: Shareef Abdoul-Raheem
// File:   ast_processor.rs
//

use crate::AstNodeLiteral;
use crate::AstNodeRoot;
use crate::AstNodeTag;
use crate::AstNodeText;

pub trait IAstProcessor {
  fn has_error(&mut self) -> bool;
  fn visit_begin_root(&mut self, root_node: &AstNodeRoot) -> ();
  fn visit_begin_tag(&mut self, tag_node: &AstNodeTag) -> ();
  fn visit_text(&mut self, text_node: &AstNodeText) -> ();
  fn visit_literal(&mut self, literal_node: &AstNodeLiteral) -> ();
  fn visit_end_tag(&mut self, tag_node: &AstNodeTag) -> ();
  fn visit_end_root(&mut self, root_node: &AstNodeRoot) -> ();
}
