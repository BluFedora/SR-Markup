
#include "sr-blog-gen.h"

#include <iostream>

extern "C" 
{
  #if 0
  __declspec(dllexport) void* srBlogGenInit(void)
  {
    std::cout << "srBlogGenInit" << std::endl;
    return nullptr;
  }
  #endif

  __declspec(dllexport) void srBlogGenProcess(const ASTNode* node, void* user_data)
  {
    std::cout << "srBlogGenProcess" << std::endl;

    switch (node->tag)
    {
      case ASTNode_TagNode:
      {
        node->tag_node.text;

        printf(
        "TAG Node(%.*s)\n", 
        int(node->tag_node.text.str_end - node->tag_node.text.str_start), 
        node->tag_node.text.str_start);
        break;
      }
      case ASTNode_TextNode:
      {
        printf(
          "Text Node(%.*s)\n", 
          int(node->text_node.text.str_end - node->text_node.text.str_start), 
          node->text_node.text.str_start);
        break;
      }
      case ASTNode_LiteralNode:
      {
        printf("Literal Node\n");
        break;
      }
    };
  }

  __declspec(dllexport) void srBlogGenShutdown(void* user_data)
  {
    std::cout << "srBlogGenShutdown" << std::endl;
  }
}