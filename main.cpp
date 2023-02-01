/*
 * Copyright [2022] <Copyright yourui>
 * @Author: yourui
 * @Date: 2023-1-13
 */
#include <iostream>
#include "AhoCorasick.hpp"

void test1() {
  std::vector<std::string> keys = {"ab", "c", "a", "acd"};
  std::string input = "dabcacdfc";
  AhoCorasick::AhoCorasick myAC;
  myAC.addStrVec(keys);
  myAC.createTrie();
  std::vector<AhoCorasick::AhoMatch> results = myAC.findAllTexts(input);
  //std::sort(results.begin(), results.end(),[](AhoCorasick::AhoMatch a, AhoCorasick::AhoMatch b){return a.pos < b.pos;});

  // debug
  myAC.printTrie();
  myAC.printRes(keys, input, results);
}

int main () {
  test1();
}