#include <iostream>

#include "binary_tree.h"

int main() {

    BinaryTree bt;

    bt.Insert(1);

    bt.Insert(15);

    bt.Insert(2);
    bt.Insert(3);
    bt.Insert(4);

    bt.Delete(3);

    bt.InorderWalk();

    return 0;
}