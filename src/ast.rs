//
// Author: Shareef Abdoul-Raheem
// File:   ast.rs
//

use std::collections::HashMap;

// AST Nodes

pub type ASTNodePtr = Box<ASTNode>;
pub type ASTNodeList = Vec<ASTNodePtr>;

/// A parsed document will have exactly one root ast node.
pub struct ASTNodeRoot {
    pub children: ASTNodeList,
}

#[derive(Debug)]
pub enum ASTNodeLiteral {
    Str(String),
    Float(f64),
    Bool(bool),
}

impl ToString for ASTNodeLiteral {
    fn to_string(&self) -> String
    {
        match self {
            ASTNodeLiteral::Str(value) => value.clone(),
            ASTNodeLiteral::Float(value) => value.to_string(),
            ASTNodeLiteral::Bool(value) => value.to_string(),
        }
    }
}

/// main building block for the document, can be nested and have key value pair of extra metadata.
pub struct ASTNodeTag {
    pub text: String,
    pub children: ASTNodeList,
    pub attributes: HashMap<String, ASTNodeLiteral>,
}

impl ASTNodeTag {
    pub fn find_attribute(self: &Self, key:&str) -> Option<&ASTNodeLiteral> 
    {
        return self.attributes.get(key);
    }
}

pub struct ASTNodeText {
    pub text: String,
}

pub enum ASTNode {
    Root(ASTNodeRoot),
    Tag(ASTNodeTag),
    Text(ASTNodeText),
    Literal(ASTNodeLiteral), // TODO(SR): See if this can be removed.
}

impl ASTNodeTag {
    pub fn new(text: String) -> Self {
        Self {
            text,
            children: Default::default(),
            attributes: Default::default(),
        }
    }
}
