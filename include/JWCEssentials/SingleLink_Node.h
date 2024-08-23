// MIT License
// Copyright (c) 2024 John W. Cornell
// See LICENSE file in the project root for full license information.
#ifndef SINGLELINK_NODE_H
#define SINGLELINK_NODE_H

namespace JWCEssentials {
    template <typename T>
    class SingleLink_Node {
    public:
        T value;
        P_INSTANCE(SingleLink_Node) next = nullptr;
        P_INSTANCE(void) tag = nullptr;

        SingleLink_Node() = default;

        explicit SingleLink_Node(T value)
        {
            this->value = value;
        }

        P_INSTANCE(SingleLink_Node)  set_next(T value)
        {
            P_INSTANCE(SingleLink_Node)  R = next = new SingleLink_Node(value);
            return R;
        }

        P_INSTANCE(SingleLink_Node)  tail_add(T value) {
            P_INSTANCE(SingleLink_Node) newNode = new SingleLink_Node();
            newNode->value = value;

            P_INSTANCE(SingleLink_Node)  cur = this;
            while (cur->next) cur = cur->next;
            cur->next = newNode;

            return newNode;
        }
    };
}

#endif //SINGLELINK_NODE_H
