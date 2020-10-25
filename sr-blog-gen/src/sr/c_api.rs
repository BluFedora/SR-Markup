//
// Shareef Abdoul-Raheem
//

// Dynamic Library API

use libc::{c_char};

#[repr(C)]
pub struct StringView {
  pub str_start: *const c_char,
  pub str_end: *const c_char,
}

#[repr(C)]
pub struct ASTNodeListView {
  pub num_nodes: u32,
  pub nodes: *const ASTNodeView,
}

#[repr(C)]
pub enum ASTNodeLiteralValue {
  /// cbindgen:field-names=[value]
  AsStr(StringView),
  /// cbindgen:field-names=[value]
  AsNumber(f64),
  /// cbindgen:field-names=[value]
  AsBoolean(bool),
}

#[repr(C)]
pub struct TagAttributeView {
  pub key: StringView,
  pub value: ASTNodeLiteralValue,
}

#[repr(C)]
/// cbindgen:prefix-with-name
pub enum ASTNodeView {
  /// cbindgen:field-names=[text, children, num_attributes, attributes]
  TagNode(StringView, ASTNodeListView, u32, *const TagAttributeView),
  /// cbindgen:field-names=[text]
  TextNode(StringView),
  /// cbindgen:field-names=[value]
  LiteralNode(ASTNodeLiteralValue),
}

/*
#[repr(C)]
pub struct BlogAPI {
  user_data: *mut c_void,
  attribute_begin: extern "C" fn(node: *mut c_void),
  attribute_has_next: extern "C" fn(node: *mut c_void) -> bool,
  attribute_next: extern "C" fn(node: *mut c_void),
  get_text: extern "C" fn(node: *mut c_void),
}

#[no_mangle]
pub extern "C" fn srBlogNumChildren(node_ptr: *const c_void) -> u32 {
  let node: *const ASTNode = node_ptr as *const ASTNode;

  unsafe {
    return match *node {
      ASTNode::Root(ref node_root) => node_root.children.len(),
      ASTNode::Tag(ref node_tag) => node_tag.children.len(),
      ASTNode::Text(_) => 0,
      ASTNode::Literal(_) => 0,
    };
  }
}

#[no_mangle]
pub extern "C" fn srBlogChildAt(node_ptr: *const c_void, index: size_t) -> *const c_void {
  let node: *const ASTNode = node_ptr as *const ASTNode;

  unsafe {
    return match *node {
      ASTNode::Root(ref node_root) => {
        &*node_root.children[index] as *const ASTNode as *const c_void
      }
      ASTNode::Tag(ref node_tag) => &*node_tag.children[index] as *const ASTNode as *const c_void,
      ASTNode::Text(_) => std::ptr::null(),
      ASTNode::Literal(_) => std::ptr::null(),
    };
  }
}

#[no_mangle]
pub extern "C" fn srBlogChildrenNext(this: *const c_void) -> c_void {
  if this == std::ptr::null() {
    return 1;
  }

  return 0;
}

#[no_mangle]
pub extern "C" fn srBlogGetText(this: *const c_void) -> StringView {
  return StringView {
    str_start: std::ptr::null(),
    str_end: std::ptr::null(),
  };
}
*/