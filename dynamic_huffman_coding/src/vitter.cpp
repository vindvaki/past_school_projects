// Reiknirit Vitters fyrir kvika Huffman-kóðun
#include <cstdlib>
#include <iostream>
#include <stack>
#include <queue>
#include <vector>

class FloatingTree {
    int n,M,R,E,Z, availBlock,
        *block, *weight, *parent, *parity, *rtChild, *first, *last, *prevBlock, *nextBlock; 
    int *alpha,     // q -> j
        *rep;       // j -> q
public: 
    // tekur stærð stafrófsins sem stika
    // svarar til `Initialize`
    FloatingTree(int);
    void encodeAndTransmit(int, std::queue<int> &);
    int receiveAndDecode(std::queue<int> &);
    int findChild(int, int);
    void update(int);
private:
    void interchangeLeaves(int, int);
    void findNode(int k,int&q,int&lti,int&bq,int&b,int&op1,int&op2,int&nbq,int&par,int&bpar);
    void slideAndIncrement(int&q,int&lti,int&bq,int&b,int&op1,int&op2,int&nbq,int&par,int&bpar,bool&);
};

int main(int argc, char *argv []) {
    int limit = (argc==1)?(-1):(std::atoi(argv[1])); // stafir sem lesa skal
    int size = 256;
    FloatingTree encoder(size);
    FloatingTree decoder(size);
    unsigned char c;  // mikilvægt!!!
    int inp_size = 0,
        enc_size = 0;
    while((inp_size != limit) && (std::cin >> std::noskipws >> c)) {
        std::queue<int> transfer;
        encoder.encodeAndTransmit(c+1, transfer);
        encoder.update(c+1);
        inp_size++;
        enc_size += transfer.size();
        /* 
        // Skrifa út
        int dec = decoder.receiveAndDecode(transfer);
        decoder.update(dec);
        std::cout << char(dec-1);
        */
    }
    std::cout << "Chars: " << inp_size << std::endl;
    std::cout << "Bits:  " << enc_size << std::endl;
    return 0;
}

FloatingTree::FloatingTree(int size) {
    n = size;
    M = 0; E = 0; R = -1; Z = 2*n - 1;
    // númerum allt frá 1
    alpha = new int[n+1]; rep = new int[n+1];
    for(int i = 1; i <= n; i++) {
        M++; R++;
        if(2*R == M) { E++; R = 0; }
        alpha[i] = i; rep[i] = i;
    }
    block = new int[Z+1]; weight = new int[Z+1]; parent = new int[Z+1];
    parity = new int[Z+1]; rtChild = new int[Z+1]; first = new int[Z+1];
    last = new int[Z+1]; prevBlock = new int[Z+1]; nextBlock = new int[Z+1];
    // initialize node n as the 0-node
    block[n] = 1; prevBlock[1] = 1; nextBlock[1] = 1; weight[1] = 0;
    first[1] = n; last[1] = n; parity[1] = 0; parent[1] = 0;
    // initialize available block list
    availBlock = 2;
    for(int i = availBlock; i <= Z-1; i++)
        nextBlock[i] = i+1;
    nextBlock[Z] = 0;
}
void FloatingTree::encodeAndTransmit(int j, std::queue<int> &out) {
    int q = rep[j];
    std::stack<bool> stack;
    if(q <= M) {
        // Encode letter of zero weight
        q--;
        int t;
        if(q < 2*R)
            t = E+1;
        else {
            q -= R;
            t = E;
        }
        /* 
        // Bætir aukabitum á hlaðann til að tákna stafinn.
        // Nauðsynlegt til að geta afkóðað, en þessir bitar
        // eru ekki taldir með í mælingum
        for(int i = 0; i != t; i++) {
            stack.push(q%2);
            q /= 2;
        }
        */
        q = M;
    }
    int root = (M == n)?n:Z;
    while(q != root) {
        // Traverse up the tree
        stack.push((first[block[q]]- q + parity[block[q]])%2);
        q = parent[block[q]]- (first[block[q]]- q + 1 - parity[block[q]])/2;
    }
    // senda kóðann fyrir stafinn
    while(!stack.empty()) {
        out.push(stack.top());
        stack.pop();
    }
}

int FloatingTree::receiveAndDecode(std::queue<int> &in) {
    int q = (M==n)?n:Z; // set q to the root node
    int receive;
    while(q > n) {
        receive = in.front(); in.pop();
        q = findChild(q, receive);
    }
    if(q == M) { // decode 0-node
        q = 0;
        for(int i = 0; i != E; i++) {
            receive = in.front(); in.pop();
            q = 2*q + receive;
        }
        if(q < R) {
            receive = in.front(); in.pop();
            q = 2*q + receive;
        }
        else
            q += R;
        q++;
    }
    return alpha[q];
}

int FloatingTree::findChild(int j, int _parity) {
    int delta = 2*(first[block[j]] - j) + 1 - _parity,
        right = rtChild[block[j]],
        gap = right - last[block[right]];
    if(delta <= gap)
        return right - delta;
    else {
        delta = delta - gap - 1;
        right = first[prevBlock[block[right]]];
        gap = right - last[block[right]];
        if(delta <= gap)
            return right-delta;
        else
            return first[prevBlock[block[right]]] - delta + gap + 1;
    }
}

void FloatingTree::interchangeLeaves(int x, int y) {
    int tmp;
    rep[alpha[x]] = y; rep[alpha[y]] = x;
    tmp = alpha[x]; alpha[x] = alpha[y]; alpha[y] = tmp;
}

void FloatingTree::update(int k) {
    int q, leafToIncrement, bq, b, oldParent, oldParity, nbq, par, bpar;
    bool slide;
    findNode(k,q,leafToIncrement,bq,b,oldParent,oldParity,nbq,par,bpar);
    while(q > 0) {
        slideAndIncrement(q,leafToIncrement,bq,b,oldParent,oldParity,nbq,par,bpar, slide);
    }
    if(leafToIncrement != 0) {
        q = leafToIncrement;
        slideAndIncrement(q,leafToIncrement,bq,b,oldParent,oldParity,nbq,par,bpar, slide);
    }
}

// viðföngin eiga að vera tilvísanir í tilviksbreytur update
void FloatingTree::findNode(
        int k, 
        int &q, int &leafToIncrement, int &bq, int &b,
        int &oldParent,int &oldParity, int &nbq, int &par, int&bpar)
{
    q = rep[k]; leafToIncrement = 0;
    if(q <= M) { // a zero weight becomes positive
        interchangeLeaves(q,M); 
        if(R == 0) {
            R = M/2;
            if(R > 0)
                E--;
        }
        M--; R--; q = M+1; bq = block[q];
        if(M > 0) {
            // Split the 0-node into an internal node with two children.
            // The new 0-node is node M, the old 0-node is node M+1; the
            // parent of nodes M and M+1 is node M+n.
            block[M] = bq; last[bq] = M; oldParent = parent[bq];
            parent[bq] = M+n; parity[bq] = 1;
            // Create a new internal block of zero weight for node M+n-1
            b = availBlock; availBlock = nextBlock[availBlock];
            prevBlock[b] = bq; nextBlock[b] = nextBlock[bq];
            prevBlock[nextBlock[bq]] = b; nextBlock[bq] = b;
            parent[b]= oldParent; parity[b] = 0; rtChild[b] = q;
            block[M+n] = b; weight[b] = 0;
            first[b] = M+n; last[b] = M+n;
            leafToIncrement = q; q = M+n;
        }
    } 
    else { // interchange q with the first node in q's block
        interchangeLeaves(q,first[block[q]]);
        q = first[block[q]];
        if( (q == M+1) && (M > 0) ) {
            leafToIncrement = q;
            q = parent[block[q]];
        }
    }
}
void FloatingTree::slideAndIncrement( 
        int &q, int &leafToIncrement, int &bq, int &b, int &oldParent, 
        int &oldParity, int &nbq, int &par, int &bpar, bool &slide)
{
    // q is currently the first node in its block
    bq = block[q]; nbq = nextBlock[bq];
    par = parent[bq]; oldParent = par; oldParity = parity[bq];
    if(((q <= n) && (first[nbq] > n) && (weight[nbq] == weight[bq]))
            || ((q > n) && (first[nbq] <= n) && (weight[nbq] == weight[bq]+1)))
    { // slide q over the next block
        slide = true;
        oldParent = parent[nbq]; oldParity = parity[nbq];
        // adjust child pointers for next higher level in tree
        if(par > 0) {
            bpar = block[par];
            if(rtChild[bpar] == q)
                rtChild[bpar] = last[nbq];
            else if(rtChild[bpar] == first[nbq])
                rtChild[bpar] = q; 
            else 
                rtChild[bpar]++;
            if(par != Z) {
                if(block[par+1] != bpar) {
                    if(rtChild[block[par+1]] == first[nbq])
                        rtChild[block[par+1]] = q;
                    else if(block[rtChild[block[par+1]]] == nbq)
                        rtChild[block[par+1]]++;
                }
            }
        }
        // adjust parent pointers for block nbq
        parent[nbq] = parent[nbq]-1+parity[nbq]; parity[nbq] = 1-parity[nbq];
        nbq = nextBlock[nbq];
    }
    else slide = false;
    if( (((q <= n) && (first[nbq] <= n)) || ((q > n) && (first[nbq] > n)))
            && (weight[nbq] == weight[bq]+1))
    { // merge q into the block of weight one higher
        block[q] = nbq; last[nbq] = q;
        if(last[bq] == q) {
            // q's old block disappears
            nextBlock[prevBlock[bq]] = nextBlock[bq];
            prevBlock[nextBlock[bq]] = prevBlock[bq];
            nextBlock[bq] = availBlock; availBlock = bq;
        }
        else {
            if(q > n)
                rtChild[bq] = findChild(q-1, 1);
            if(parity[bq] == 0)
                parent[bq]--;
            parity[bq] = 1-parity[bq];
            first[bq] = q-1;
        }
    }
    else if(last[bq] == q) {
        if(slide) {
            // q's block is slid forward in the block list
            prevBlock[nextBlock[bq]] = prevBlock[bq];
            nextBlock[prevBlock[bq]] = nextBlock[bq];
            prevBlock[bq] = prevBlock[nbq]; nextBlock[bq] = nbq;
            prevBlock[nbq] = bq; nextBlock[prevBlock[bq]] = bq;
            parent[bq] = oldParent; parity[bq] = oldParity;
        }
        weight[bq]++;
    }
    else {
        // a new block is created for q
        b = availBlock; availBlock = nextBlock[availBlock];
        block[q] = b; first[b] = q; last[b] = q;
        if(q > n) {
            rtChild[b] = rtChild[bq];
            rtChild[bq] = findChild(q-1, 1);
            if(rtChild[b] == q-1)
                parent[bq] = q;
            else if(parity[bq] == 0)
                parent[bq]--;
        }
        else if(parity[bq] == 0)
            parent[bq]--;
        first[bq] = q-1; parity[bq] = 1-parity[bq];
        // insert q's block in its proper place in the block list
        prevBlock[b] = prevBlock[nbq]; nextBlock[b] = nbq;
        prevBlock[nbq] = b; nextBlock[prevBlock[b]] = b;
        weight[b] = weight[bq]+1;
        parent[b] = oldParent; parity[b] = oldParity;
    }
    // move q one higher level in the tree
    if(q <= n) 
        q = oldParent;
    else
        q = par;
}
