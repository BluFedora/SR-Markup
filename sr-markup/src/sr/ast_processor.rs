//
// Author: Shareef Abdoul-Raheem
// File:   ast_processor.rs
//

use crate::ASTNodeLiteral;
use crate::ASTNodeRoot;
use crate::ASTNodeTag;
use crate::ASTNodeText;

pub trait IASTProcessor {
  fn has_error(&mut self) -> bool;
  fn visit_begin_root(&mut self, root_node: &ASTNodeRoot) -> ();
  fn visit_begin_tag(&mut self, tag_node: &ASTNodeTag) -> ();
  fn visit_text(&mut self, text_node: &ASTNodeText) -> ();
  fn visit_literal(&mut self, literal_node: &ASTNodeLiteral) -> ();
  fn visit_end_tag(&mut self, tag_node: &ASTNodeTag) -> ();
  fn visit_end_root(&mut self, root_node: &ASTNodeRoot) -> ();
}
