/*
 * Copyright [2022] <Copyright yourui>
 * @Author: yourui
 * @Date: 2023-2-1
 */
#ifndef AHO_CORASICK_
#define AHO_CORASICK_
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

namespace AhoCorasick {

//
const int MAX_CHILD_NODE_NUM = 256;
const int MAX_TEXT_NUM = 50;

// AhoText
struct AhoText {
  int id;
  std::string text;
  size_t size;
  AhoText(int i, std::string t, size_t s) : id(i), text(t), size(s) {}
};

// AhoMatch
struct AhoMatch {
  AhoMatch(int i, int pos, int size) : id(i), pos(pos), size(size) {}
  int id;
  int pos;
  int size;
};

// AhoNode
class AhoNode {
 public:
  AhoNode(const char c)
      : text(c), child_num(0), text_end(false), ref_count(1), parent_node(nullptr), failure_node(nullptr) {
    // initialize child_nodes
    for (int i = 0; i < MAX_CHILD_NODE_NUM; i++) {
      child_node[i] = nullptr;
    }
  }

  ~AhoNode() {
    // fprintf(stderr, "~AhoNode!\n");
    for (size_t i = 0; i < child_num; i++) {
      if (child_node[i] != nullptr) {
        delete child_node[i];
      }
    }
  }

 public:
  char text;
  size_t child_num;
  bool text_end;
  int ref_count;
  std::unordered_map<char, AhoNode*> child_map;

  AhoText* output_text;
  // Parent node
  AhoNode* parent_node;
  // Child nodes
  AhoNode* child_node[MAX_CHILD_NODE_NUM];
  // Failure link nodes
  AhoNode* failure_node;
  AhoNode* output_node;
};

// AhoTrie
class AhoTrie {
 public:
  AhoTrie() {
    root = new AhoNode('0');
    root->failure_node = root;
  }
  ~AhoTrie() {
    delete (root);
  }
  // addTrieNodeFromText
  bool addTrieNodeFromText(AhoText& ahotext) {
    AhoNode* t_node = root;
    std::string text = ahotext.text;
    for (size_t i = 0; i < text.size(); i++) {
      char node_char = text[i];
      bool find_node = false;
      int child_idx = 0;

      unsigned child_num = t_node->child_num;
      if (child_num == 0) {
        // init first child with node_char
        AhoNode* first_child_node = new AhoNode(node_char);
        first_child_node->parent_node = t_node;
        t_node->child_node[0] = first_child_node;
        t_node->child_map[node_char] = first_child_node;
        t_node->child_num++;
        // update t_node with the first child node
        t_node = first_child_node;
        continue;
      }

      if (child_num == MAX_CHILD_NODE_NUM) {
        fprintf(stderr, "[addTrieNodeFromText] child_num reach to MAX_CHILD_NODE_NUM = %d!\n", MAX_CHILD_NODE_NUM);
        return false;
      }

      for (child_idx = 0; child_idx < child_num; child_idx++) {
        if (t_node->child_node[child_idx]->text == node_char) {
          find_node = true;
          break;
        }
      }

      if (find_node == true) {
        t_node->child_node[child_idx]->ref_count++;  // increment
        t_node = t_node->child_node[child_idx];
      } else {
        // not find then add new child index equal child_num
        AhoNode* new_child_node = new AhoNode(node_char);
        t_node->child_node[child_num] = new_child_node;
        t_node->child_map[node_char] = new_child_node;
        t_node->child_num++;
        new_child_node->parent_node = t_node;
        // update t_node with the new child node
        t_node = new_child_node;
      }
    }
    if (t_node != nullptr) {
      t_node->text_end = true;
      t_node->output_text = &ahotext;
    }
    return true;
  }
  // connectLinkHandler
  bool connectLinkHandler(AhoNode* parent, AhoNode* child) {
    AhoNode* parent_failure = nullptr;
    int i = 0;
    // node root
    if (parent->failure_node == nullptr || parent->parent_node == nullptr) {
      child->failure_node = parent;
      return true;
    }

    parent_failure = parent->failure_node;
    int child_num = parent_failure->child_num;
    for (int i = 0; i < child_num; i++) {
      if (parent_failure->child_node[i]->text == child->text) {
        // connect failure link
        child->failure_node = parent_failure->child_node[i];
        // connect output link
        if (parent_failure->child_node[i]->text_end) {
          child->output_node = parent_failure->child_node[i];
        } else {
          child->output_node = parent_failure->child_node[i]->output_node;
        }
        return true;
      }
    }
    return false;
  }
  // connectLink
  bool connectLink() {
    std::queue<AhoNode*> q;
    q.push(root);
    // BFS
    while (!q.empty()) {
      AhoNode* child_node = nullptr;
      AhoNode* queue_node = q.front();
      q.pop();
      int child_num = queue_node->child_num;
      for (int i = 0; i < child_num; i++) {
        AhoNode* parent_node = queue_node;
        child_node = parent_node->child_node[i];
        q.push(child_node);

        while (!connectLinkHandler(parent_node, child_node)) {
          parent_node = parent_node->failure_node;
        }
      }
    }
    return true;
  }

  // findTrieNodeHandler
  bool findTrieNodeHandler(AhoNode** t_node, const char text) {
    AhoNode* s_node = *t_node;
    if (s_node == nullptr) return false;
    int s_child = s_node->child_num;
    for (int i = 0; i < s_child; i++) {
      if (s_node->child_node[i]->text == text) {
        *t_node = s_node->child_node[i];
        return true;
      }
    }
    return false;
  }
  // findTrieNodeText
  AhoText* findTrieNodeText(AhoNode** t_node, const char text) {
    while (!findTrieNodeHandler(t_node, text)) {
      if (*t_node == nullptr || (*t_node)->parent_node == nullptr) return nullptr;
      *t_node = (*t_node)->failure_node;
    }
    // find text end!
    if ((*t_node)->text_end) return (*t_node)->output_text;

    // find output link
    if ((*t_node)->output_node) return (*t_node)->output_node->output_text;

    // next node
    return nullptr;
  }

  AhoNode* findTrieNode(AhoNode** t_node, const char text) {
    while (!findTrieNodeHandler(t_node, text)) {
      if (*t_node == nullptr || (*t_node)->parent_node == nullptr) return nullptr;
      *t_node = (*t_node)->failure_node;
    }
    // find text end!
    if ((*t_node)->text_end || (*t_node)->output_node) return *t_node;

    // next node
    return nullptr;
  }

  // printTrie
  void printTrie() {
    std::queue<AhoNode*> q;
    q.push(root);
    while (!q.empty()) {
      AhoNode* node = q.front();
      q.pop();
      for (int i = 0; i < node->child_num; i++) {
        q.push(node->child_node[i]);
      }
      if (node == root) {
        printf("root node %p\n", node);
        continue;
      }
      printf("%c (textend:%d) node %p ref %u (parent %p) failure_node(%p) output_node(%p) ", node->text, node->text_end,
             node, node->ref_count, node->parent_node, node->failure_node, node->output_node);
      if (node->text_end) {
        printf("[id: %d, text: %s, size: %zu]", node->output_text->id, node->output_text->text.c_str(),
               node->output_text->size);
      }
      printf("\n");
    }
  }

 public:
  AhoNode* root;
};

// AhoCorasick
class AhoCorasick {
 public:
  AhoCorasick() {
    trie = new AhoTrie();
    texts_id = 0;
  }
  ~AhoCorasick() {
    delete (trie);
  }

  bool addStr(const std::string& text) {
    if (texts_id >= MAX_TEXT_NUM) {
      fprintf(stderr, "Too much text in AhoCorasick!\n");
      return false;
    }
    texts.push_back(AhoText(texts_id, text, text.size()));
    texts_id++;
    return true;
  }

  bool addText(const char* text, unsigned int text_length) {
    if (texts_id >= MAX_TEXT_NUM + 1) {
      fprintf(stderr, "Too much text in AhoCorasick!\n");
      return false;
    }
    texts.push_back(AhoText(texts_id, text, text_length));
    texts_id++;
    return true;
  }

  bool addStrVec(const std::vector<std::string>& textvec) {
    for (int i = 0; i < textvec.size(); i++) {
      if (!addStr(textvec[i])) return false;
    }
    return true;
  }

  // createTrie
  bool createTrie() {
    for (int i = 0; i < texts.size(); i++) {
      if (!trie->addTrieNodeFromText(texts[i])) {
        fprintf(stderr, "Add trie node error! | text: %s\n", texts[i].text.c_str());
      }
    }
    trie->connectLink();
    return true;
  }

  // findTexts
  std::vector<AhoMatch> findTexts(const std::string& data) {
    int data_len = data.length();
    std::vector<AhoMatch> results;
    AhoNode** t_node = &(trie->root);
    for (int i = 0; i < data_len; i++) {
      AhoText* result;
      result = trie->findTrieNodeText(t_node, data[i]);
      if (!result) {
        continue;
      }
      AhoMatch match(result->id, i - result->size + 1, result->size);
      results.push_back(match);
    }
    return results;
  }

  // findAllTexts
  std::vector<AhoMatch> findAllTexts(const std::string& data) {
    int data_len = data.length();
    std::vector<AhoMatch> results;
    AhoNode** t_node = &(trie->root);
    for (int i = 0; i < data_len; i++) {
      AhoNode* resultNode;
      resultNode = trie->findTrieNode(t_node, data[i]);
      if (!resultNode) {
        continue;
      }
      // 匹配上输出
      if (resultNode->text_end) {
        AhoText* text = resultNode->output_text;
        AhoMatch match(text->id, i - text->size + 1, text->size);
        results.push_back(match);
      }
      // 匹配上右子序列输出
      while (resultNode->output_node) {
        AhoText* text = resultNode->output_node->output_text;
        AhoMatch match(text->id, i - text->size + 1, text->size);
        results.push_back(match);
        resultNode = resultNode->output_node;
      }
    }
    return results;
  }

  // printTrie
  void printTrie() {
    trie->printTrie();
  }

  // printResult
  void printRes(std::vector<std::string>& keys, std::string& input, std::vector<AhoMatch>& result) {
    printf("KEYs :[");
    for (int i = 0; i < keys.size() - 1; i++) {
      printf("%s, ", keys[i].c_str());
    }
    printf("%s]\n", keys[keys.size() - 1].c_str());

    printf("INPUT: %s\n", input.c_str());

    for (int i = 0; i < result.size(); i++) {
      printf("\t match text: %s (match id: %d position: %d length: %d)\n", keys[result[i].id].c_str(), result[i].id,
             result[i].pos, result[i].size);
    }
  }
  //
 private:
  AhoTrie* trie;
  std::vector<AhoText> texts;
  int texts_id;
};

}  // namespace AhoCorasick
#endif  // AHO_CORASICK_
