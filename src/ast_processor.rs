//
// Author: Shareef Abdoul-Raheem
// File:   ast_processor.rs
//

use crate::ast::ASTNode;
use crate::ast::ASTNodeLiteral;
use crate::ast::ASTNodeRoot;
use crate::ast::ASTNodeTag;
use crate::ast::ASTNodeText;

#[derive(PartialEq)]
pub enum ASTProcessorVisitResult {
    Continue,
    SkipChildren,
    Halt,
}

/// Helper Interface for traversing an SRMark AST.
pub trait IASTProcessor {
    fn visit_begin_root(&mut self, root_node: &ASTNodeRoot) -> ASTProcessorVisitResult;
    fn visit_begin_tag(&mut self, tag_node: &ASTNodeTag) -> ASTProcessorVisitResult;
    fn visit_text(&mut self, text_node: &ASTNodeText) -> ASTProcessorVisitResult;
    fn visit_literal(&mut self, literal_node: &ASTNodeLiteral) -> ASTProcessorVisitResult;
    fn visit_end_tag(&mut self, tag_node: &ASTNodeTag) -> ();
    fn visit_end_root(&mut self, root_node: &ASTNodeRoot) -> ();
}

pub fn visit_ast(node: &ASTNode, processor: &mut dyn IASTProcessor) -> ASTProcessorVisitResult {
    let mut continue_processing = ASTProcessorVisitResult::Continue;
    match node {
        ASTNode::Root(r) => {
            if processor.visit_begin_root(r) == ASTProcessorVisitResult::Continue {
                for child in &r.children {
                    continue_processing = visit_ast(&child, processor);

                    if continue_processing == ASTProcessorVisitResult::Halt {
                        break;
                    }
                }

                processor.visit_end_root(r);
            }
        }
        ASTNode::Tag(t) => {
            if processor.visit_begin_tag(t) == ASTProcessorVisitResult::Continue {
                for child in &t.children {
                    continue_processing = visit_ast(&child, processor);

                    if continue_processing == ASTProcessorVisitResult::Halt {
                        break;
                    }
                }

                processor.visit_end_tag(t);
            }
        }
        ASTNode::Text(t) => {
            continue_processing = processor.visit_text(t);
        }
        ASTNode::Literal(l) => {
            continue_processing = processor.visit_literal(l);
        }
    }

    return continue_processing;
}
