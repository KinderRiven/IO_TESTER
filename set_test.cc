#include <cstdio>
#include <set>
#include <stdint.h>
#include <string.h>
using namespace std;

#define KEY_LENGTH (16)

struct KVPair {
public:
    char key[KEY_LENGTH];
    uint64_t offset;

public:
    bool operator<(const KVPair& k) const
    {
        return memcmp(this->key, k.key, KEY_LENGTH) < 0 ? true : false;
    }
};

static char array_keys[4][KEY_LENGTH] = { "12345678", "11111111", "00000000", "22222222" };

int main()
{
    set<KVPair> itr;
    KVPair pair;
    for (int i = 0; i < 4; i++) {
        memcpy(pair.key, array_keys[i], KEY_LENGTH);
        itr.insert(pair);
    }
    printf("%zu\n", itr.size());
    for (auto i = itr.begin(); i != itr.end(); i++) {
        printf("%s\n", i->key);
    }
    return 0;
}
