#include <iostream>
#include <cassert>
#include "proto_test.pb.h"

using namespace std;

int main(int argc, char** argv) {
    proto_test::Union u;
    
    // 玩家1
    auto p1 = u.add_players();
    p1->set_age(21);
    p1->set_male(0);
    p1->set_name("player1");
    p1->mutable_food()->set_name("青椒肉丝");

    // 玩家2
    auto p2 = u.add_players();
    p2->set_age(35);
    p2->set_male(1);
    p2->set_name("player2");

    auto s = u.SerializeAsString();
    assert(!s.empty());

    proto_test::Union u_copy;
    u_copy.ParseFromString(s);
    for(int i = 0; i < u_copy.players_size(); ++i) {
        auto& p = u_copy.players(i);
        if(p.has_food()) {
            printf("name:%s,age:%d,male:%d,food:%s\r\n", p.name().c_str(), p.age(), p.male(), p.food().name().c_str());
        } else {
            printf("name:%s,age:%d,male:%d\r\n", p.name().c_str(), p.age(), p.male());
        }
    }
    return 0;
}