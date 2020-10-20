//
// Shareef Abdoul-Raheem
//

// Dynamic Library API

// GetType() -> c_int
// (ASTNodeRoot, ASTNodeTag) Child Begin, Child Next, Child End / HasNext
// (ASTNodeTag, ASTNodeText) GetText

use crate::sr::ast::ASTNode;
// use crate::sr::ast::ASTNodeList;

use libc::{c_char, c_int, c_void, size_t};

#[repr(C)]
pub struct StringView {
  str_start: *const c_char,
  str_end: *const c_char,
}

#[repr(C)]
pub struct ASTNodeListView {
  num_nodes: size_t,
  nodes: *const ASTNodeView,
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
  key: StringView,
  value: ASTNodeLiteralValue,
}

#[repr(C)]
/// cbindgen:prefix-with-name
pub enum ASTNodeView
{
  /// cbindgen:field-names=[children]
  Root(ASTNodeListView),
  /// cbindgen:field-names=[text, children, num_attributes, attributes]
  Tag(StringView, ASTNodeListView, size_t, * const TagAttributeView),
  /// cbindgen:field-names=[text]
  Text(StringView),
  /// cbindgen:field-names=[value]
  Literal(ASTNodeLiteralValue),
}

#[repr(C)]
pub struct BlogAPI {
  user_data: *mut c_void,
  attribute_begin: extern "C" fn(node: *mut c_int),
  attribute_has_next: extern "C" fn(node: *mut c_int) -> bool,
  attribute_next: extern "C" fn(node: *mut c_int),
  get_text: extern "C" fn(node: *mut c_int),
}

#[no_mangle]
pub extern "C" fn srBlogNumChildren(node_ptr: *const c_void) -> size_t {
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
      ASTNode::Root(ref node_root) => &*node_root.children[index] as *const ASTNode as *const c_void,
      ASTNode::Tag(ref node_tag) =>  &*node_tag.children[index]as *const ASTNode as *const c_void,
      ASTNode::Text(_) => std::ptr::null(),
      ASTNode::Literal(_) => std::ptr::null(),
    };
  }
}

#[no_mangle]
pub extern "C" fn srBlogChildrenHasNext(this: *const c_void) -> c_int {
  if this == std::ptr::null() {
    return 1;
  }

  return 0;
}

#[no_mangle]
pub extern "C" fn srBlogChildrenNext(this: *const c_void) -> c_int {
  if this == std::ptr::null() {
    return 1;
  }

  return 0;
}

#[no_mangle]
pub extern "C" fn srBlogGetText() -> StringView {
  return StringView {
    str_start: std::ptr::null(),
    str_end: std::ptr::null(),
  };
}
