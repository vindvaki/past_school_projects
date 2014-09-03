#include <iostream>
#include <queue>
#include <map>
#include <vector>
#include <cstdlib>

using namespace std;

class node {
public:
    node *left, *right;
    char data; int weight;

    // build leaf
    node(char d, int f) {
        left = 0; right = 0;
        data = d; weight = f;
    }
    // build internal node
    node(node *lnode, node *rnode) {
        left = lnode; right = rnode;
        weight = lnode->weight + rnode->weight;
    }
    ~node() {
        if(left) { 
            delete left;
            delete right;
        }
    }
};
// for node comparison in the priority queue
struct cmp {
    bool operator() (const node *a, const node *b) const {
        return a->weight > b->weight;
    }
};

typedef map<char,vector<bool> > encoding_table;

// Huffman tré úr tíðnitöflu
node *make_tree(map<char,int> &);
// Kóðunartafla úr Huffman-tré
void make_encoding_table(node *, encoding_table &, vector<bool> &);

int main(int argc, char *argv []) {
    int limit = (argc==1)?(-1):(atoi(argv[1])); // stafir sem lesa skal
    map<char,int> freq;
    char c;
    queue<char> input;
    while((input.size() != limit) && (cin >> noskipws >> c)) {
        input.push(c);
        freq[c]++;
    }
    // kóða strenginn
    node *htree = make_tree(freq); encoding_table enc; vector<bool> prefix;
    make_encoding_table(htree, enc, prefix);
    queue<bool> encoding;

    cout << "Alpha: " << freq.size() << endl;
    cout << "Chars: " << input.size() << endl;

    while(!input.empty()) {
        vector<bool> b = enc[input.front()]; input.pop();
        for(int i = 0; i != b.size(); i++)
            encoding.push(b[i]);
    }
    cout << "Bits:  " << encoding.size() << endl;
    /*
    // afkóða strenginn
    while(!encoding.empty()) {
        node *path = htree;
        while(path->left) {
            bool b = encoding.front();
            path = (b)?(path->right):(path->left);
            encoding.pop();
        }
        cout << path->data;
    }
    */
    return 0;
}

node *make_tree(map<char,int> &freq) {
    priority_queue<node *, vector<node*>, cmp> q;
    // laufin
    for(map<char,int>::iterator it = freq.begin(); it != freq.end(); it++) {
        node *z = new node(it->first, it->second);
        q.push(z);
    }
    // innri hnútar
    while(q.size() != 1) {
        node *x = q.top(); q.pop();
        node *y = q.top(); q.pop();
        node *z = new node(x,y);
        q.push(z);
    }
    return q.top(); // rótin
}

void make_encoding_table(node *htree, encoding_table &table, vector<bool> &prefix) {
    if(htree->left) {
        prefix.push_back(0);
        make_encoding_table(htree->left, table, prefix);
        prefix.back() = 1;
        make_encoding_table(htree->right, table, prefix);
        prefix.pop_back();
    }
    else {
        table[htree->data] = prefix;
    }
}

