#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>
#include <map>
#include <set>
#include <unordered_set>
#define PI 3.14
#define TK 1.0f
using namespace std;
struct Node{
public:
    Node(double dst):_dist_from_centre(dst){
    }
    double tick_to_new_car=2e9;
    bool Tick(){
        curr_time++;
        if(curr_time>=tick_to_new_car){
            curr_time-=tick_to_new_car;
            return true;
        }
        return false;
    }
    double _dist_from_centre;
private:
    double curr_time=0;
};
struct Road{
public:
    Road(Node* from_, Node* to_, double distance, double coef=1):from(from_),to(to_),_distance(distance),_traffic_coef(coef){

    }
    double GetTime(){return _time;}
    void RecalculateData(){
        _speed = (1/_traffic_coef)*min(_max_speed,10000/(_popularity+0.01));
        _time = _distance/_speed;
    }
    double GetVelocity(){return _speed;}
    double GetDistance(){ return _distance;}
    Node* from = nullptr;//start node
    Node* to = nullptr;//destination node
    double _popularity=0;//количество машин на трассе
private:
    double _speed=0;
    double _max_speed=90;
    double _time=0;//время на преодоление
    double _distance=0;//длина дороги
    double _traffic_coef=0;//магическая постоянная
};
struct Unit{
public:
    Unit(Node* finish, Node* start):_destination(finish),_local_destination(start){}
    bool IsFinish(){ return finished;}
    Node* CurrentDestination(){
        return _local_destination;
    }
    Node* Destination(){
        return _destination;
    }
    Node* CurrentFrom(){
        return _local_from;
    }
    void Tick(Road* next_road,double timestep=TK){
        total_time+=timestep;
        if(finished){
            return;
        }
        if(_location== nullptr){
            _location=next_road;
            _local_from = _location->from;
            _local_destination=_location->to;
            _dist_remain = _location->GetDistance();
            return;
        }
        _dist_remain-=timestep*_location->GetVelocity();
        if(_dist_remain<0){
            if(_local_destination==_destination){
                finished=true;
                return;
            }
            _location=next_road;
            _local_from = _location->from;
            _local_destination=_location->to;
            _dist_remain = _location->GetDistance();
        }
    }
    Road* CurrentRoad(){
        return _location;
    }
    double total_time = 0;
private:
    bool finished = false;
    double _dist_remain = 0;
    Node* _local_from = nullptr;
    Node* _destination;//точка, куды едем
    Node* _local_destination;//точка текущего прибытия по дороге
    Road* _location= nullptr;//дорога, на которой находимся
};
struct City{
    friend int main();
public:
    map<Node*,map<Node*, Road*>> RecalculateShortestPath(){
        //работает на слегка модернизированном Флойде-Уоршалле
        //TODO: модифицировать на рандомный выбор
        map<Node*,map<Node*,Road*>> answer;//первая дорога на кратчайшем пути из 1 ноды во 2
        map<Node*,map<Node*, double>> algorithm_answer;//мин растояние
        //начальная инициализация бесконечностями
        map<Node*,double> algorithm_dummy;
        map<Node*, Road*> answer_dummy;
        for(auto init_node:_nodes){
            algorithm_dummy[init_node]=2e9;//заполнение бесконечностями
            answer_dummy[init_node] = nullptr;//заполнение дефолтами
        }
        for(auto init_node:_nodes){
            answer[init_node]=answer_dummy;
            algorithm_answer[init_node]=algorithm_dummy;
        }
        //начальная инициализация рёбрами
        for(Node* first_node:_nodes) {
            for (Node* second_node:_nodes) {
                if(first_node==second_node){
                    algorithm_answer[first_node][second_node]=0;
                    continue;
                }
                if(_roads.find(first_node)!=_roads.end()){
                    if(_roads[first_node].find(second_node)!=_roads[first_node].end()){
                        //пересчитать время в пути перед использованием
                        _roads.at(first_node).at(second_node).RecalculateData();
                        algorithm_answer.at(first_node).at(second_node) = _roads.at(first_node).at(second_node).GetTime();
                        answer.at(first_node).at(second_node) = &(_roads.at(first_node).at(second_node));
                    }
                }
            }
        }
        for(auto current_node_in_middle:_nodes){
            for(auto first_node:_nodes) {
                for (auto second_node:_nodes) {
                    //проверка обнаружения более короткого пути
                    auto debb = algorithm_answer[first_node][second_node];
                    auto bedd1 = algorithm_answer[first_node][current_node_in_middle];
                    auto bedd2 = algorithm_answer[current_node_in_middle][second_node];
                    if (algorithm_answer[first_node][second_node] >
                        algorithm_answer[first_node][current_node_in_middle] + algorithm_answer[current_node_in_middle][second_node]){
                        algorithm_answer[first_node][second_node] =
                                algorithm_answer[first_node][current_node_in_middle] + algorithm_answer[current_node_in_middle][second_node];
                        auto deb = answer[first_node][current_node_in_middle];
                        answer[first_node][second_node] = answer[first_node][current_node_in_middle];
                    }
                }
            }
        }
        return answer;
    }
    //добавления структур
    void AddCircle(double radius, double traffic_coef=1){
        CircleRadiuses.push_back(radius);
        Node* firstRadialNode = new Node(radius);
        Node* prevNode = firstRadialNode;
        _nodes.push_back(firstRadialNode);
        for(int i=1; i<sectors;++i){
            Node* currNode = new Node(radius);
            _nodes.push_back(currNode);
            //не забыть создать обратную дорогу
            _roads[prevNode].insert({currNode, Road(prevNode,currNode,2*PI*radius/sectors,traffic_coef)});
            _roads[currNode].insert({prevNode, Road(currNode,prevNode,2*PI*radius/sectors,traffic_coef)});
            prevNode = currNode;
        }
        Road cost(_nodes[_nodes.size()-1],firstRadialNode,2*PI*radius/sectors,traffic_coef);
        Road cost2(firstRadialNode,_nodes[_nodes.size()-1],2*PI*radius/sectors,traffic_coef);
        _roads[_nodes[_nodes.size()-1]].insert({firstRadialNode, cost});
        _roads[firstRadialNode].insert({_nodes[_nodes.size()-1],cost2});
    }
    void AddRadial(int nodesBetweenRadius, double trafficConst=1){
        node_between_radius = nodesBetweenRadius;
        double max_radius = CircleRadiuses[CircleRadiuses.size()-1];
        for(int i=1; i<CircleRadiuses.size();++i){
            for(int j=0; j<sectors;++j){
                //рисование одного радиуса
                Node* start = _nodes[(i-1)*sectors+j];
                Node* finish = _nodes[i*sectors+j];
                double segmentLength = (CircleRadiuses[i]-CircleRadiuses[i-1])/(nodesBetweenRadius+1);
                if(nodesBetweenRadius!=0) {
                    Node* firstBetweenNode = new Node((CircleRadiuses[i-1]+segmentLength));
                    _nodes.push_back(firstBetweenNode);
                    Node* lastNode=firstBetweenNode;
                    //не забыть создать обратную дорогу
                    _roads[start].insert({firstBetweenNode, Road(start,firstBetweenNode,segmentLength,trafficConst)});
                    _roads[firstBetweenNode].insert({start, Road(firstBetweenNode,start,segmentLength,trafficConst)});
                    for (int k=1;k<nodesBetweenRadius;++k ){
                        Node* newNode = new Node(CircleRadiuses[i-1] + segmentLength*(k+1));
                        _nodes.push_back(newNode);
                        //не забыть создать обратную дорогу
                        _roads[lastNode].insert({newNode, Road(lastNode,newNode,segmentLength,trafficConst)});
                        _roads[newNode].insert({lastNode, Road(newNode,lastNode,segmentLength,trafficConst)});
                        lastNode = newNode;
                    }
                    //не забыть создать обратную дорогу
                    _roads[lastNode].insert({finish, Road(lastNode,finish,segmentLength,trafficConst)});
                    _roads[finish].insert({lastNode, Road(finish,lastNode,segmentLength,trafficConst)});
                }else{
                    _roads[start].insert({finish, Road(start,finish,segmentLength,trafficConst)});
                    _roads[finish].insert({start, Road(finish,start,segmentLength,trafficConst)});
                }
            }
        }
    }
    //скорее всего понадобятся функции брания n-й вершины диаметра/кольца
    //отсчёт с 0
    Node* takeNRadialNode(int RadiusNumber, int OrderNumber){
        int radialSector = OrderNumber/(node_between_radius+1);//номер ближайшего радиуса
        int orderInSector = OrderNumber%(node_between_radius+1);//отступ от него
        if(orderInSector==0){
            return _nodes[sectors*(radialSector)+RadiusNumber];
        }
        int xxx = sectors*CircleRadiuses.size()-1+node_between_radius*RadiusNumber*(CircleRadiuses.size()-1) + node_between_radius*radialSector+orderInSector;
        return _nodes[sectors*CircleRadiuses.size()-1+node_between_radius*RadiusNumber*(CircleRadiuses.size()-1) + node_between_radius*radialSector+orderInSector];
        //return _nodes[node_between_radius*CircleRadiuses.size()*RadiusNumber+node_between_radius*radialSector+orderInSector-1];
    }
private:
    int sectors=4;//число диаметров
    vector<double> CircleRadiuses;
    int node_between_radius;
    deque<Node*> _nodes;
    map<Node*,map<Node*,Road>> _roads;
    friend class Model;
};
struct Model{
public:
    void TimeTick(){
        //сгенерить новых машин
        for(int sector=0; sector<_city.sectors;++sector){
            for(int num_from_center =0; num_from_center<((_city.CircleRadiuses.size()-1)*(_city.node_between_radius+1)+1);
            num_from_center++){
                Node* from = _city.takeNRadialNode(sector,num_from_center);
                if(from->Tick()){
                    Node* to = _city.takeNRadialNode((sector+_city.sectors/2)%_city.sectors,num_from_center);
                    _cars.insert(new Unit(from,to));
                }
            }
        }
        //переситать популярность дорог
        for(auto& firstNodeIterator: _city._roads){
            for(auto& tempRoad: firstNodeIterator.second){
                tempRoad.second._popularity=0;
            }
        }
        for(Unit* currCar:_cars){
            if(currCar->CurrentRoad()!=nullptr) {
                currCar->CurrentRoad()->_popularity += 1;
            }
        }
        //просчитать кратчайшие пути
        auto FirstRoadInShortestPath = _city.RecalculateShortestPath();
        //сдвинуть все машины
        vector<Unit*> deleteCandidates;
        for(Unit* CurrentCar: _cars){
            CurrentCar->Tick(FirstRoadInShortestPath[CurrentCar->CurrentDestination()][CurrentCar->Destination()]);
            if(CurrentCar->IsFinish()){
                deleteCandidates.push_back(CurrentCar);
            }
        }
        //выпилить прибывших
        for(auto i:deleteCandidates){
            sum_time+=i->total_time;
            car_amount+=1;
            _cars.erase(i);
            delete i;
        }
        //вывести инфу
        cout<<"==================\n";
        if(car_amount>1){
            cout<<sum_time << "\n" <<car_amount<<"\n"<< sum_time/car_amount<<"\n";
        }
        cout<<_cars.size()<<"\n";
    }
    void Run(){
        for(Node* nd:_city._nodes){
            nd->tick_to_new_car=_city.CircleRadiuses[_city.CircleRadiuses.size()-1]/nd->_dist_from_centre;
        }
        while(true){
            TimeTick();
        }
    }
    City _city;
private:
    unordered_set<Unit*> _cars;
    double sum_time=0;
    double car_amount=0;
};
int main() {
    Model model;
    model._city.AddCircle(100.0f,3);
    model._city.AddCircle(300.0f, 1);
    model._city.AddRadial(1);
    auto deb = model._city.takeNRadialNode(0,3);
    model.Run();

    /*
    City c;
    c._nodes.push_back(new Node(0));
    c._nodes.push_back(new Node(100));
    c._nodes.push_back(new Node(200));
    c._roads[c._nodes[0]].insert({c._nodes[1],Road(c._nodes[0],c._nodes[1],100)});
    c._roads[c._nodes[1]].insert({c._nodes[0],Road(c._nodes[1],c._nodes[0],100)});
    c._roads[c._nodes[1]].insert({c._nodes[2],Road(c._nodes[1],c._nodes[2],100)});
    c._roads[c._nodes[2]].insert({c._nodes[1],Road(c._nodes[2],c._nodes[1],100)});
    auto x = c.RecalculateShortestPath();
    */
     return 0;
}