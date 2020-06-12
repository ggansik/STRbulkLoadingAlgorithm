#include <iostream>
#include <vector>
#include <list>
#include <cmath>
#include <algorithm>
using namespace std;

typedef std::vector<int64_t> Coordinates;

// 하나의 subarray를 나타내기 위한 클래스
class skylineCandidate
{
public:
    Coordinates coordinate;
    std::vector<double> value; //f_1,...,f_k에 대한 aggregate 결과 값

    skylineCandidate() = default;
};
class Node;
class NodeEntry;
class RTree;

class RTree{
public:
    Node* root = nullptr;

    RTree()=default;

    template <typename T>
    vector<Node*> bulkLoading(vector<T>& MBRs, vector<size_t>& currentSubarrays, size_t k, size_t r, size_t b);
    void bulkLoading_onlyLeaf(const vector<skylineCandidate>& subarrays, size_t k, size_t r, size_t b);

};
class MBR{ // == MBR
public:
    Coordinates value; //MBR의 center coordinate
    Node* p;
    list<size_t> idx_subarrays; // leaf entry일때만 non-empty
};
class Node{
public:
    list<MBR> entries;
    vector<Node*> child;
    bool isLeaf = false;
};

template <typename T>
vector<Node*> RTree::bulkLoading(vector<T>& MBRs, vector<size_t>& currentSubarrays, size_t k, size_t r, size_t b){
    //slice the data space so that each slice contains sliceSize = b * ceil(ceil(r/b)^((k-1)/k)) objects.
    //Each slice is now processed recursively using the k = k-1 and r = sliceSize.
    cout << "k == " << k << endl;
    cout << endl;

    size_t p = ceil((double)r/(double)b); //최종 leaf page가 몇개가 되는지?
    size_t slice = ceil(pow((double)p, 1/(double)k)); //현재 attribute에서 몇개로 자를건지
    size_t sliceSize = b * ceil(pow((double)p,(((double)k-1.0)/(double)k))); //slice개로 자르면 각 slice마다 몇개가 들어가는지
    size_t dim = MBRs[0].value.size(); //전체 dimension 수

    cout << slice <<" slices"<< endl;
    cout << "each slice has " << sliceSize << " objects" << endl;

    //현재 attribute기준으로 내가 가진 subarray index 정렬
    vector<pair<double, size_t>> sortedIdx;
    for(size_t subarrIdx : currentSubarrays){
        sortedIdx.emplace_back(make_pair(MBRs[subarrIdx].value[dim - k], subarrIdx));
    }
    std::sort(sortedIdx.begin(), sortedIdx.end());

    //아래 레벨에 전달할 슬라이스별로 subarray idx를 저장하는 과정
    vector<vector<size_t>> slicedIdx; //슬라이스별로 subarray idx를 저장하기 위한 벡터
    vector<size_t> temp; //슬라이스 하나에 해당하는 임시벡터
    for(size_t i = 0, subarrCnt = 0; i < sortedIdx.size(); i++, subarrCnt++) {
        if(subarrCnt == sliceSize){ //가득차있으면 비우고 새로
            subarrCnt = 0;
            slicedIdx.push_back(temp);
            temp = {};
        }
        temp.push_back(sortedIdx[i].second);
    }
    if(temp.size() > 0) slicedIdx.push_back(temp); //마지막 슬라이스는 sliceSize 보다 적을수 있음

    /*
    cout << "MBRs for each slice:" << endl;
    size_t num = 0;
    for(vector<size_t> Aslice : slicedIdx){
        cout << num << ": ";
        for(size_t subIdx : Aslice) cout << subIdx << " ";
        cout << ", length " << Aslice.size();
        cout << endl;
        num += 1;
    }
     */
    cout << "------------------------------------------" << endl;
    if(k == dim){
        vector<Node*> tempMBRs;
        for(size_t sliceNum = 0; sliceNum < slicedIdx.size(); sliceNum++){
            cout << "subarrays: " << slicedIdx[sliceNum].size() << endl;
            tempMBRs.emplace_back(RTree::bulkLoading(MBRs, slicedIdx[sliceNum],k-1, slicedIdx[sliceNum].size(), b)[0]);
        }
        MBR temp;
        vector<MBR> nextData;
        vector<size_t> currentMBRIdx;
        for(size_t i = 0; i < tempMBRs.size(); i++){
            temp.value = {}; //이거 계산해야댐
            temp.p = tempMBRs[i];
            nextData.emplace_back(temp);
            currentMBRIdx.push_back(i);
        }
        RTree::bulkLoading(nextData, currentMBRIdx, k, currentMBRIdx.size(), b);
    }else if(k > 1) {
        //leaf node가 아닐 경우 각 slice에 대해서 bulkLoading을 호출한다.
        vector<Node*> tempMBRs;
        for(size_t sliceNum = 0; sliceNum < slicedIdx.size(); sliceNum++){
            cout << "subarrays: " << slicedIdx[sliceNum].size() << endl;
            tempMBRs.emplace_back(RTree::bulkLoading(MBRs, slicedIdx[sliceNum],k-1, slicedIdx[sliceNum].size(), b)[0]);
        }
        return tempMBRs;
    }else{//leaf node 이면
        //leaf node인 경우(k == 1)
        //slicedIdx의 subarrayIdx들을 묶어서 NodeEntry(MBR)로 만들고 Node로 묶어서 리턴
        vector<Node*> temp;
        Node* node = new Node;
        node->child = {};
        node->isLeaf = true;
        for(size_t sliceNum = 0; sliceNum < slicedIdx.size(); sliceNum++){
            MBR temp;
            Coordinates entryCoord = {}; //이거 구하는 방법?
            temp.value = entryCoord;
            temp.p = nullptr; //리프노드의 MBR이므로 가리키는 노드가 없다
            for(size_t idx : slicedIdx[sliceNum]){
                temp.idx_subarrays.push_back(idx); //엔트리의 서브어레이 넣기
            }
            node->entries.emplace_back(temp);
            //temp 초기화...?
        }
        temp.emplace_back(node);
        return temp;
    }
}

/*
Node* RTree::bbulkLoading(const vector<skylineCandidate>& subarrays, vector<size_t>& currentSubarrays, size_t k, size_t r, size_t b){
    //slice the data space so that each slice contains sliceSize = b * ceil(ceil(r/b)^((k-1)/k)) objects.
    //Each slice is now processed recursively using the k = k-1 and r = sliceSize.
    cout << "k == " << k << endl;
    cout << endl;

    size_t p = ceil((double)r/(double)b); //최종 leaf page가 몇개가 되는지?
    size_t slice = ceil(pow((double)p, 1/(double)k)); //현재 attribute에서 몇개로 자를건지
    size_t sliceSize = b * ceil(pow((double)p,(((double)k-1.0)/(double)k))); //slice개로 자르면 각 slice마다 몇개가 들어가는지
    size_t dim = subarrays[0].value.size(); //전체 dimension 수

    cout << slice <<" slices"<< endl;
    cout << "each slice has " << sliceSize << " objects" << endl;

    //현재 attribute기준으로 내가 가진 subarray index 정렬
    vector<pair<double, size_t>> sortedIdx;
    for(size_t subarrIdx : currentSubarrays){
        sortedIdx.emplace_back(make_pair(subarrays[subarrIdx].value[dim - k], subarrIdx));
    }
    std::sort(sortedIdx.begin(), sortedIdx.end());

    //아래 레벨에 전달할 슬라이스별로 subarray idx를 저장하는 과정
    vector<vector<size_t>> slicedIdx; //슬라이스별로 subarray idx를 저장하기 위한 벡터
    vector<size_t> temp; //슬라이스 하나에 해당하는 임시벡터
    for(size_t i = 0, subarrCnt = 0; i < sortedIdx.size(); i++, subarrCnt++) {
        if(subarrCnt == sliceSize){ //가득차있으면 비우고 새로
            subarrCnt = 0;
            slicedIdx.push_back(temp);
            temp = {};
        }
        temp.push_back(sortedIdx[i].second);
    }
    if(temp.size() > 0) slicedIdx.push_back(temp); //마지막 슬라이스는 sliceSize 보다 적을수 있음

    cout << "subarrays for each slice:" << endl;
    size_t num = 0;
    for(vector<size_t> Aslice : slicedIdx){
        cout << num << ": ";
        for(size_t subIdx : Aslice) cout << subIdx << " ";
        cout << ", length " << Aslice.size();
        cout << endl;
        num += 1;
    }
    cout << "------------------------------------------" << endl;
    vector<Node*> nodes; //하위레벨의 노드들을 저장하는 벡터(얘네를 묶으면 현재노드가 됨)
    if(k > 1) {
        //leaf node가 아닐 경우 각 slice에 대해서 bulkLoading을 호출한다.
        for(size_t sliceNum = 0; sliceNum < slicedIdx.size(); sliceNum++){
            cout << "subarrays: " << slicedIdx[sliceNum].size() << endl;
            nodes.emplace_back(RTree::bulkLoading(subarrays, slicedIdx[sliceNum], k-1, slicedIdx[sliceNum].size(), b));
        }
        Node* node = new Node;
        node->child = {};
        for(size_t i = 0; i < nodes.size(); i++){
            NodeEntry temp;
            Coordinates entryCoord = {};
            temp.coordinate = entryCoord;
            temp.p = nodes[i];
            node->child.emplace_back(nodes[i]); //이거 해야됨? NodeEntry의 p랑 child 둘 다 필요한가?
            temp.idx_subarrays = {};

            node->entries.emplace_back(temp);
        }
        return node;
    }else{
        //leaf node인 경우(k == 1)
        //slicedIdx의 subarrayIdx들을 묶어서 NodeEntry(MBR)로 만들고 Node로 묶어서 리턴
        Node* node = new Node;
        node->child = {};
        node->isLeaf = true;
        for(size_t sliceNum = 0; sliceNum < slice; sliceNum++){
            NodeEntry temp;
            Coordinates entryCoord = {};
            temp.coordinate = entryCoord;
            temp.p = nullptr; //리프노드의 엔트리이므로 가리키는 노드가 없다
            for(size_t idx : slicedIdx[sliceNum]){
                temp.idx_subarrays.push_back(idx); //엔트리의 서브어레이 넣기
            }
            node->entries.emplace_back(temp);
            //temp 초기화...?
        }
        return node;
    }
}
*/
void RTree::bulkLoading_onlyLeaf(const vector<skylineCandidate> &subarrays, size_t k, size_t r, size_t b) {}

int main() {

    vector<skylineCandidate> subarrays;
    skylineCandidate temp;
    for(size_t i = 0; i < 5; i++){
        for(size_t j = 0; j < 5; j++){
            temp.value = {double(i), double(j)};
            subarrays.push_back(temp);

        }
    }

    //Sort-Tile-Recursive algorithm.
    //input
    // k: dimension of dataset.
    // r: the number of data objects.
    // b: number of MBRs that one node can contain(fanout).
    RTree rtree;
    size_t k = subarrays[0].value.size();
    size_t r = subarrays.size();
    size_t b = 3;

    vector<size_t> index_subarrays;
    for(size_t index=0; index<subarrays.size(); index++){
        index_subarrays.push_back(index);
    }
    cout << "the number of dimensions: " << k << endl;
    cout << "the number of data objects: " << r << endl;
    cout << "fanout of the rtree: " << b << endl;
    cout << endl;
    cout << "------------------------------------------" << endl;

    rtree.root = rtree.bulkLoading(subarrays, index_subarrays, k, r, b);

    return 0;
}
/*
void RTree::bbulkLoading(const vector<skylineCandidate>& subarrays, size_t k, size_t r, size_t b)
{

    //a hyper rectangle is defined by k intervals of the form [A_i,B_i]
    //slice the data space so that each slice contains new_r = b * ceil(ceil(r/b)^((k-1)/k)) objects.
    //Each slice is now processed recursively using the k = k-1 and r = new_r.
    size_t new_r = b * ceil(pow(ceil(r/b),((k-1)/k)));
    bulkLoading(subarrays, k-1, new_r, b);
    // 1. subarrays를 z-address의 오름차순으로 정렬한다.
    vector<pair<boost::dynamic_bitset<>,size_t>> sortedIdx; // z-address, index
    for(size_t idx=0; idx<subarrays.size(); idx++){
        sortedIdx.emplace_back(subarrays[idx].bits, idx);
    }
    std::sort(sortedIdx.begin(), sortedIdx.end());
//        LOG4CXX_DEBUG(logger,"Sorted subarrays by z-address");
//        for(const pair<boost::dynamic_bitset<>,size_t>& pair: sortedIdx){
//            LOG4CXX_DEBUG(logger,"Z-address: "<<pair.first<<", idx: "<<pair.second);
//        }

    // 2. leaf node를 형성하기 위한 부분
    LOG4CXX_DEBUG(logger,"-----Leaf Level Start-----");
    vector<ZBNode_entry> currentLevelEntries;
    size_t start;
    size_t end;
    bool initial = true;
    while(true){
        if(initial){
            start = 0;
            initial = false;
        }
        else{
            start = end + 1;
        }
        end = std::min(start + N - 1, sortedIdx.size()-1);
        if(start == sortedIdx.size()) {
            break;
        }
        ZBNode_entry thisEntry;
        thisEntry.alpha = subarrays[sortedIdx[start].second];
        thisEntry.beta = subarrays[sortedIdx[end].second];
        thisEntry.updateMinMaxPT();
        size_t smallestRZRegion = thisEntry.calculateRZRegionSize();
        for(size_t i=start; i<=smallestEnd; i++){
            thisEntry.idx_subarrays.push_back(sortedIdx[i].second);
        }
        currentLevelEntries.emplace_back(thisEntry);
    }

    // 3. leaf level의 entry들이 주어졌을 때, root에 이르기까지 ZB-tree를 구성한다.
    // entry들로 node 구성 -> 다음 레벨의 entry 생성
    int level = 1;
    vector<ZBNode_entry> nextLevelEntries;
    bool leafLevel = true;
    while(true){
        LOG4CXX_DEBUG(logger,"-----"<<level++<<"-th Level Start-----");

        end = -1;
        initial = true;
        while(true){
            if(initial){
                start = 0;
                initial = false;
            }
            else{
                start = end + 1;
            }
            end = std::min(start + N - 1, currentLevelEntries.size()-1);
            if(start == currentLevelEntries.size()) {
                break;
            }
//                LOG4CXX_DEBUG(logger,"Initial start: " << start<<", end: "<< end);

            size_t smallestEnd = end;
            size_t possibleRange = std::min(start+M-1, end);
//                LOG4CXX_DEBUG(logger,"end's possible range: "<<possibleRange<<"~"<<end);
            ZBNode_entry thisEntry;
            thisEntry.alpha = currentLevelEntries[start].alpha;
            thisEntry.beta = currentLevelEntries[end].beta;
            thisEntry.updateMinMaxPT();
            size_t smallestRZRegion = thisEntry.calculateRZRegionSize();
//                LOG4CXX_DEBUG(logger,"Initial alpha: "<<thisEntry.alpha.address<<", beta: "<<thisEntry.beta.address
//                    <<", minPT: "<<thisEntry.minPT.address<<", maxPT: "<<thisEntry.maxPT.address<<", RZ-region size: "<<smallestRZRegion);

//                LOG4CXX_DEBUG(logger,"New node has been created");
            auto* newNode = new ZBNode();
            for(size_t i=start; i<=smallestEnd; i++){
//                    LOG4CXX_DEBUG(logger,"entry's alpha: "<<currentLevelEntries[i].alpha.address<<", beta: "<<currentLevelEntries[i].beta.address);
                newNode->entries.emplace_back(currentLevelEntries[i]);
            }
            thisEntry.p = newNode;
            nextLevelEntries.emplace_back(thisEntry);

            if(leafLevel){
                newNode->isLeaf = true;
            }
//                LOG4CXX_DEBUG(logger,"Selected start: " << start<<", Selected end: "<< end);
        }
        if(leafLevel){
            leafLevel = false;
        }

        if(nextLevelEntries.size() < N){ // 트리 생성 종료 조건
//                LOG4CXX_DEBUG(logger,"-----Root Level Start-----");
            root = new ZBNode();
            for(const ZBNode_entry& entry: nextLevelEntries){
                root->entries.emplace_back(entry);
            }
            ZBNode_entry tempEntry;
            tempEntry.alpha = root->entries.begin()->alpha;
            tempEntry.beta = root->entries.back().beta;
            tempEntry.updateMinMaxPT();
            root->minPT = tempEntry.minPT;
            root->maxPT = tempEntry.maxPT;
//                LOG4CXX_DEBUG(logger,"root's entries: "<<root->entries.size());
//                for(const ZBNode_entry& entry: root->entries){
//                    LOG4CXX_DEBUG(logger,"entry's alpha: "<<entry.alpha.address<<", beta: "<<entry.beta.address);
//                }
            return; // 완료
        }

        currentLevelEntries.clear();
        for(const ZBNode_entry& entry: nextLevelEntries){
            currentLevelEntries.emplace_back(entry);
        }
        nextLevelEntries.clear();
    }
}
 */