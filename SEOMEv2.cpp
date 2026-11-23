#include <iostream>     // Used
#include <string>       // Used
#include <fstream>      // Used
#include <sstream>      // Used
#include <cstdlib>      // Used
#include <cctype>       // Used
#include <ctime>        // Used
#include <math.h>
#include <cmath>
#include <limits>
#include <iomanip>
#include <vector>       // Used
#include <stdexcept>    // Used
#include <algorithm>    // Used
//Custom Header Files
#include <utils.h>      // Used
using namespace std;

// Forward Class Declarations
class Account;
class User;
class Order;
class Trade;
class TradeLog;
class OrderQueue;
class OrderHeap;
class HashTable;
class MatchingEngine;
class OrderManager;
class FileHandler;
class StockExchange;

// OrderType Enumeration
enum class OrderType{ BUY, SELL };

// Account Module
class Account{
    private:
    double balance;
    public:
    Account(double initializationAmount = 0.0): balance(initializationAmount) {}
    double getBal(){ return balance; }
    void deposit(double amount){ 
        if(amount <= 0){
            throw runtime_error("Deposit Amount Must be Positive!\n");
        }
        balance += amount; 
    }
    double withdraw(double withD){
        if(withD <= 0){
            throw runtime_error("Withdrawal Amount Must be Positive!\n");
        }
        if(withD > balance) {
            throw runtime_error("Insufficient Funds!\n");
        }
        balance -= withD;
        return balance;
    }
};

// User Module
class User{
    private:
    int userID;
    string name;
    string pass;
    Account account;
    static int userCounter;
    public:
    // Basic Constructor
    User(string n = "Guest", string password = "123", double initialBalance = 0.0): userID(userCounter++), name(n), pass(password), account(initialBalance) {}
    // Restoring Constructor
    User(int id, string n, string password, double balance): userID(id), name(n), pass(password), account(balance){
        if(userCounter <= id) userCounter = id + 1;
    }
    int getUserID(){ return userID; }
    string getName(){ return name; }
    string getPassword(){ return pass; }
    double getUserBal() { return account.getBal(); }
    void userDeposit(double amount){ account.deposit(amount); }
    double userWithdraw(double withD){ return account.withdraw(withD); }
};
// Static Members Declarations (SMD)
int User::userCounter = 1;

// Represents an Individual Order
class Order{
    private:
    int orderID;
    User* user;
    
    double price;
    int shares;
    int remainingShares;
    
    int sequenceNumber;
    bool active;
    
    static int idCounter;
    static int sequenceCounter;
    
    public:
    OrderType type;
    Order(User* user, 
        OrderType type,
        double price, 
        int shares
    ):  user(user),
        type(type),
        price(price),
        shares(shares),
        sequenceNumber(sequenceCounter++),
        orderID(idCounter++),
        remainingShares(shares),
        active(true) {}
    Order(User* user, 
        OrderType type, 
        double price,
        int shares, 
        int id, 
        int sequence,
        int remaining, 
        bool isActive
    ): user(user),
      type(type),
      price(price),
      shares(shares),
      sequenceNumber(sequence),
      orderID(id),
      remainingShares(remaining),
      active(isActive) {
        if(idCounter <= id)
            idCounter = id + 1;
        if(sequenceCounter <= sequence)
            sequenceCounter = sequence + 1;
    }
    bool isActive() const { return active == true; }
    void reduceShares(int decShares) {
        if(decShares > remainingShares || decShares <= 0){
            throw runtime_error("Reduced Share Amount must not be greater than Remaining Shares!\n");
        }
        remainingShares -= decShares;
    }
    void cancelOrder() { active = false; }
    void displayOrder() const {
        cout << "Order ID: "<<(type == OrderType::BUY ? "B" : "S")<<orderID << endl;
        cout << "Type: " << (type == OrderType::BUY ? "BUY" : "SELL") << endl;
        cout << "Price: " << price << endl;
        cout << "Original Shares: " << shares << endl;
        cout << "Remaining Shares: " << remainingShares << endl;
    }
    int getOrderID() const { return orderID; }
    double getPrice() const { return price;}
    int getRemainingShares() const { return remainingShares; }
    int getSequenceNumber() const { return sequenceNumber; }
    int getShares() const { return shares;} // Not Used Currently (NUC)
    User* getUser() const { return user;} // Not Used Currently (NUC)
};
// Static Members Declarations (SMD)
int Order::idCounter = 100;
int Order::sequenceCounter = 1;

// // Represents an Executed Trade
class Trade{
    private:
    int tradeID;
    Order* buyOrderID;
    Order* sellOrderID;
    double executionPrice;
    int tradedShares;
    time_t executionTime;
    static int tradeCounter;
    public:
    Trade(Order* buy,
        Order* sell,
        double price,
        int quantity
    ):  buyOrderID(buy),
        sellOrderID(sell),
        tradedShares(quantity),
        executionPrice(price),
        executionTime(time(nullptr)),
        tradeID(tradeCounter++) {}
    Trade(Order* buy, 
        Order* sell, 
        double price,
        int quantity, 
        int id, 
        time_t timestamp
    ) : buyOrderID(buy),
        sellOrderID(sell),
        tradedShares(quantity),
        executionPrice(price),
        executionTime(timestamp),
        tradeID(id) {
        if(tradeCounter <= id)
            tradeCounter = id + 1;
    }
    int getTradeID() const { return tradeID; } // (NUC)
    Order* getBuyOrder() const {return buyOrderID; } // (NUC)
    Order* getSellOrder() const {return sellOrderID; } // (NUC)
    double getExecutionPrice() const { return executionPrice; } // (NUC)
    int getTradedShares() const { return tradedShares; } // (NUC)
    time_t getExecutionTime() const { return executionTime; } // (NUC)
    void displayTrade() const {
        cout<<"Trade ID: T"<<tradeID<<endl;
        cout<<"Buy Order: B"<<buyOrderID->getOrderID()<<endl;
        cout<<"Sell Order: S"<<sellOrderID->getOrderID()<<endl;
        cout<<"Execution Price: "<<executionPrice<<endl;
        cout<<"Traded Shares: "<<tradedShares<<endl;
    }
};
// Static Members Declarations (SMD)
int Trade::tradeCounter = 1; 

// Logs of All Trades
class TradeLog{
    private:
    // Private Node Structure (PNS)
    struct Node{
        public:
        Trade* trade;
        Node* next;
        Node(Trade* trade): trade(trade), next(nullptr) {}
    };
    public:
    Node* head;
    TradeLog(){ head = nullptr; }
    ~TradeLog() {
        Node* temp;
        while (head != nullptr) {
            temp = head;
            head = head->next;
            delete temp->trade; // Deletes Trade Object as well
            delete temp;
        }
    }
    bool isEmpty(){ return head == nullptr; }
    void push_front(Trade* trade){
        Node* newNode = new Node(trade);
        newNode->next = head;
        head = newNode;
    }
    void display(){
        if(isEmpty()){
            printf("No Trades Available Yet!\n");
            return;
        }
        Node* temp = head;
        while(temp != nullptr){
            temp->trade->displayTrade();
            temp = temp->next;
        }
    }
    vector<Trade*> getAllTrades() {
        vector<Trade*> trades;
        Node* temp = head;
        while(temp != nullptr){
            trades.push_back(temp->trade);
            temp = temp->next;
        }
        return trades;
    }
};

// Incoming Order Queue
class OrderQueue{
    private:
    // Private Node Structure (PNS)
    struct Node{
        public:
        Order* order;
        Node* next;
        Node(Order* order): order(order), next(nullptr){}
    };
    Node* front;
    Node* rear;
    public:
    // Constructor
    OrderQueue(){ front = rear = nullptr; }
    // Destructor
    ~OrderQueue(){ while(!isEmpty()){ 
        try{
            dequeue();
        } catch(runtime_error &e){
            cout<<"Error Occured: "<<e.what()<<endl;
        } 
    }}
    bool isEmpty(){ return front == nullptr; }
    void enqueue(Order* order){
        Node* newNode = new Node(order);
        if(isEmpty()){
            front = rear = newNode;
        } else {
            rear->next = newNode;
            rear = newNode;
        }
    }
    Order* dequeue(){
        if(isEmpty())
            throw runtime_error("OrderQueue is Empty!\n");
        Node* temp = front;
        Order* result = temp->order;
        front = front->next;
        if(front == nullptr)
            rear = nullptr;
        delete temp;
        return result;
    }
};

// Buy and Sell Priority Heaps
class OrderHeap{
    private:
        Order** heap;
        int capacity;
        int heapSize;
        OrderType type;
        // Check Higher Priority
        bool hasHigherPriority(Order* a, Order* b) const {
            if(a->getPrice() != b->getPrice()){
                if(type == OrderType::BUY)
                    return a->getPrice() > b->getPrice();
                return a->getPrice() < b->getPrice();    
            }
            return a->getSequenceNumber() < b->getSequenceNumber();
        }
        // Resize Heap (Doubles the Size)
        void resize(){
            capacity *= 2;
            Order** newHeap = new Order*[capacity];
            for(int i = 0; i < heapSize; i++){
                newHeap[i] = heap[i];
            }
            delete[] heap;
            heap = newHeap;
        }
    public:
    // Constructor
    OrderHeap(
        OrderType heapType,
        int initialCapacity = 10
    ) : capacity(initialCapacity),
        heapSize(0),
        type(heapType) {
            heap = new Order*[capacity];
    }
    // Destructor
    ~OrderHeap(){ delete[] heap; }
    void insertOrder(Order* order){
        if(heapSize == capacity)
            resize();
        heap[heapSize] = order;
        heapifyUp(heapSize);
        heapSize++;
    }
    Order* removeTop(){
        if(isEmpty())
            throw runtime_error("Heap is Empty!\n");
        Order* removed = heap[0];
        heapSize--;
        if(heapSize > 0){
            heap[0] = heap[heapSize];
            heapifyDown(0);
        }
        return removed;
    }
    Order* getTop() const {
        if (isEmpty())
            throw runtime_error("Heap is Empty!\n");
        return heap[0];
    }
    void heapifyUp(int index){
        while(index > 0){
            int parent = (index - 1) / 2;
            if(hasHigherPriority(heap[index], heap[parent])){
                swap(heap[index], heap[parent]);
                index = parent;
            } else {
                break;
            }
        }
    }
    void heapifyDown(int index){
        while(true){
            int left = (2 * index) + 1;
            int right = (2 * index) + 2;
            int highest = index;
            if(left < heapSize && hasHigherPriority(heap[left], heap[highest]))
                highest = left;
            if(right < heapSize && hasHigherPriority(heap[right], heap[highest]))
                highest = right;
            if(highest != index){
                swap(heap[index], heap[highest]);
                index = highest;
            } else {
                break;
            }
        }
    }
    bool isEmpty() const { return heapSize == 0; }
    // int getSize() const { return heapSize; } // (NUC)
};

// Order ID Lookup for cancelation of Order's
class HashTable {
private:
    // Private Node Structure (PNS)
    struct Node {
        Order* order;
        Node* next;
        Node(Order* o) {
            order = o;
            next = nullptr;
        }
    };
    static const int SIZE = 100;
    Node* table[SIZE];
    int hashFunction(int orderID) const {
        return orderID % SIZE;
    }
public:
    // Constructor
    HashTable(){
        for(int i = 0; i < SIZE; i++){ table[i] = nullptr; }
    }
    ~HashTable() {
        for(int i = 0; i < SIZE; i++) {
            Node* temp = table[i];
            while(temp != nullptr) {
                Node* next = temp->next;
                delete temp;
                temp = next;
            }
            table[i] = nullptr;
        }
    }
    // Hash Table Insertion
    void insert(Order* order){
        int index = hashFunction(order->getOrderID());
        Node* newNode = new Node(order);
        newNode->next = table[index];
        table[index] = newNode;
    }
    // Hash Table Search
    Order* search(int orderID) const {
        int index = hashFunction(orderID);
        Node* temp = table[index];
        while(temp != nullptr){
            if(temp->order->getOrderID() == orderID){
                return temp->order;
            }
            temp = temp->next;
        }
        return nullptr;
    }
    // Hash Table Remove
    // Removes the hashtable node only
    bool remove(int orderID){
        int index = hashFunction(orderID);
        Node* temp = table[index];
        Node* prev = nullptr;
        while (temp != nullptr) {
            if (temp->order->getOrderID() == orderID) {
                if (prev == nullptr) { table[index] = temp->next; }
                else { prev->next = temp->next; }
                delete temp;
                return true;
            }
            prev = temp;
            temp = temp->next;
        }
        return false;
    }
    // Hash Table Display
    void display() const {
        for (int i = 0; i < SIZE; i++) {
            Node* temp = table[i];
            while (temp != nullptr) {
                temp->order->displayOrder();
                temp = temp->next;
            }
        }
    }
};

// Matches buy, sell orders & Stores Trade Logs
class MatchingEngine{
    private:
    OrderHeap* buyHeap;
    OrderHeap* sellHeap;
    public:
    TradeLog* tradeLog;
    MatchingEngine(){
        buyHeap = new OrderHeap(OrderType::BUY);
        sellHeap = new OrderHeap(OrderType::SELL);
        tradeLog = new TradeLog();
    }
    ~MatchingEngine(){ 
        delete buyHeap; 
        delete sellHeap;
        delete tradeLog;
    }
    void restoreOpenOrder(Order* order){
        if(!order->isActive() || order->getRemainingShares() <= 0) return;
        if(order->type == OrderType::BUY)
            buyHeap->insertOrder(order);
        else
            sellHeap->insertOrder(order);
    }
    void processOrder(Order* order){
        // Check if Active or if 0 Shares
        if(!(order->isActive()) || order->getRemainingShares() <= 0) {
            return;
        } 
        // Check if Order is Buy Order
        if(order->type == OrderType::BUY){
            while(order->getRemainingShares() > 0 && !sellHeap->isEmpty()){
                Order* topSell = nullptr;
                try{
                    Order* topSell = sellHeap->getTop();
                } catch(runtime_error &e){
                    cout<<"Error Occured: "<<e.what()<<endl;
                }
                // If top is inactive, remove it 
                if(!topSell->isActive()){
                    try{
                        sellHeap->removeTop();
                    } catch(runtime_error &e){
                        cout<<"Error Occured: "<<e.what()<<endl;
                    }
                    continue;
                }
                // If Price Doesn't Match, Skip the Current Iteration
                if(order->getPrice() < topSell->getPrice()){
                    break;
                }
                // Skip Self Trading
                if(order->getUser() == topSell->getUser()){
                    break;
                }
                // Remove Top & Match Orders
                try{
                    sellHeap->removeTop();
                } catch(runtime_error &e){
                    cout<<"Error Occured: "<<e.what()<<endl;
                }
                matchOrders(order, topSell, OrderType::BUY);
                // Reinsert SELL if shares remain
                if ((topSell->getRemainingShares() > 0) && (topSell->isActive())){
                    sellHeap->insertOrder(topSell);
                }
            }
            // Keep remaining BUY Shares in Buy Heap
            if(order->getRemainingShares() > 0 && order->isActive()){
                buyHeap->insertOrder(order);
            }
        }
        // Check if Order is Sell Order
        else if(order->type == OrderType::SELL){
            while(order->getRemainingShares() > 0 && !(buyHeap->isEmpty())){
                Order* topBuy = nullptr;
                try{
                    Order* topBuy = buyHeap->getTop();
                } catch(runtime_error &e){
                    cout<<"Error Occured: "<<e.what()<<endl;
                }
                // If top is inactive, remove it 
                if(!topBuy->isActive()){
                    try{
                        buyHeap->removeTop();
                    } catch(runtime_error &e){
                        cout<<"Error Occured: "<<e.what()<<endl;
                    }
                    continue;
                }
                // If Price Doesn't Match, Skip the Current Iteration
                if(order->getPrice() > topBuy->getPrice()){
                    break;
                }
                // Skip Self Trading
                if(order->getUser() == topBuy->getUser()){
                    break;
                }
                // Remove Top & Match Orders
                try{
                    buyHeap->removeTop();
                } catch(runtime_error &e){
                    cout<<"Error Occured: "<<e.what()<<endl;
                }
                matchOrders(topBuy, order, OrderType::SELL);
                // Reinsert SELL if shares remain
                if ((topBuy->getRemainingShares() > 0) && (topBuy->isActive())){
                    buyHeap->insertOrder(topBuy);
                }
            }
            // Keep Remaining SELL Shares in Sell Heap
            if((order->getRemainingShares() > 0) && (order->isActive())) {
                sellHeap->insertOrder(order);
            }
        }
    }
    void matchOrders(Order* buy, Order* sell, OrderType type){
        int shareQuantity = min(buy->getRemainingShares(), sell->getRemainingShares());
        executeTrade(buy, sell, shareQuantity, type);
    }
    void executeTrade(Order* buy, Order* sell, int quantity, OrderType type){
        Trade* newTrade = new Trade(buy, sell, (type == OrderType::BUY ? sell->getPrice() : buy->getPrice()), quantity);
        try{
            buy->reduceShares(quantity);
        } catch(runtime_error &e){
            cout<<"Error Occured: "<<e.what()<<endl;
        }
        try {
            sell->reduceShares(quantity);
        } catch(runtime_error &e){
            cout<<"Error Occured: "<<e.what()<<endl;
        }        
        addTrade(newTrade);
    }
    // Add Trade to TradeLog
    void addTrade(Trade* trade){
        tradeLog->push_front(trade);
    }
};

// Handles Incoming Orders
class OrderManager{
    private:
    public:
    OrderQueue* incomingOrders;
    HashTable* orderTable;
    MatchingEngine* engine;
    vector<Order*> allOrders;
    OrderManager(){
        incomingOrders = new OrderQueue();
        orderTable = new HashTable();
        engine = new MatchingEngine();
    }
    ~OrderManager(){ 
        delete incomingOrders; 
        delete orderTable; 
        delete engine;
        for(Order* order : allOrders) {
            delete order;
        }
    }
    void restoreOrder(Order* order){
        allOrders.push_back(order);
        if(order->isActive() && order->getRemainingShares() > 0){
            orderTable->insert(order);
        }
        engine->restoreOpenOrder(order);
    }
    void placeBuyOrder(User* user, double price, int shares){
        Order* newOrder = new Order(user, OrderType::BUY, price, shares);
        allOrders.push_back(newOrder);
        incomingOrders->enqueue(newOrder);
        orderTable->insert(newOrder);
        processPendingOrders();
    }
    void placeSellOrder(User* user, double price, int shares){
        Order* newOrder = new Order(user, OrderType::SELL, price, shares);
        allOrders.push_back(newOrder);
        incomingOrders->enqueue(newOrder);
        orderTable->insert(newOrder);
        processPendingOrders();
    }
    void processPendingOrders(){
        // Keep Dequeuing Orders until Empty
        while(!incomingOrders->isEmpty()){
            Order* order = incomingOrders->dequeue();
            // Process Each Order
            engine->processOrder(order);
            if(order->getRemainingShares() == 0){
                orderTable->remove(order->getOrderID());
            }
        }
    }
    void cancelOrder(Order* order){
        if(order == nullptr || !order->isActive() || order->getRemainingShares() <= 0) {
            return;
        }
        orderTable->remove(order->getOrderID());
        order->cancelOrder();
    }
    Order* findOrder(int orderID){
        return orderTable->search(orderID);
    }
    Order* findOrderInAllOrders(int orderID){
        for(Order* order : allOrders){
            if(order->getOrderID() == orderID){
                return order;
            }
        }
        return nullptr;
    }
};

// File Handler
class FileHandler{
    private:
    // Separator Between Data in Files
    char separator = '|';
    // File Paths
    string userFilePath = "users.txt";
    string orderFilePath = "orders.txt";
    string tradeFilePath = "trades.txt";
    
    public:
    // Save Users
    void saveUsers(const vector<User*> &users){
        ofstream file(userFilePath, ios::out | ios::trunc);
        if(!file.is_open()){
            printf("Could Not Open File!\n");
            return;
        }
        for(User* user : users){
            file<<user->getUserID()<<separator
                <<user->getName()<<separator
                <<user->getPassword()<<separator
                <<user->getUserBal()<<"\n";
        }
        file.close();
    }
    // Save Orders
    void saveOrders(OrderManager* manager){
        ofstream file(orderFilePath, ios::out | ios::trunc);
        if(!file.is_open()){
            printf("Could Not Open Orders File!\n");
            return;
        }
        for(Order* order : manager->allOrders){
            file <<order->getOrderID()<<separator
                 <<order->getUser()->getUserID()<<separator
                 <<(order->type == OrderType::BUY ? "BUY" : "SELL")<<separator
                 <<order->getPrice()<<separator
                 <<order->getShares()<<separator
                 <<order->getRemainingShares()<<separator
                 <<order->getSequenceNumber()<<separator
                 <<order->isActive()<<"\n";
        }
        file.close();
    }
    // Save Trades
    void saveTrades(OrderManager* manager){
        ofstream file(tradeFilePath, ios::out | ios::trunc);
        if(!file.is_open()){
            printf("Could Not Open Trades File!\n");
            return;
        }
        vector<Trade*> trades = manager->engine->tradeLog->getAllTrades();
        for(Trade* trade : trades) {
            file << trade->getTradeID() << separator
                 << trade->getBuyOrder()->getOrderID() << separator
                 << trade->getSellOrder()->getOrderID() << separator
                 << trade->getExecutionPrice() << separator
                 << trade->getTradedShares() << separator
                 << trade->getExecutionTime() << "\n";
        }
        file.close();
    }
    // Save All Data
    void saveData(vector<User*>& users, OrderManager* manager) {
        saveUsers(users);
        saveOrders(manager);
        saveTrades(manager);
    }

    // Load Users
    void loadUsers(vector<User*> &users){
        ifstream file(userFilePath);
        if(!file.is_open()){
            return;
        }
        string line;
        while(getline(file, line)){
            stringstream ss(line);
            string id, name, password, balance;
            getline(ss, id, separator);
            getline(ss, name, separator);
            getline(ss, password, separator);
            getline(ss, balance, separator);
            User* user = new User(
                stoi(id),
                name,
                password,
                stod(balance)
            );
            users.push_back(user);
        }
        file.close();
    }
    // Load Orders
    void loadOrders(OrderManager* manager, const vector<User*> &users){
        ifstream file(orderFilePath);
        if(!file.is_open()){
            return;
        }
        string line;
        while(getline(file, line)){
            stringstream ss(line);
            string id, userID, type, price, shares, remaining, sequence, active;
            getline(ss, id, separator);
            getline(ss, userID, separator);
            getline(ss, type, separator);
            getline(ss, price, separator);
            getline(ss, shares, separator);
            getline(ss, remaining, separator);
            getline(ss, sequence, separator);
            getline(ss, active, separator);
            User* owner = nullptr;
            for(User* user : users){
                if(user->getUserID() == stoi(userID)){
                    owner = user;
                    break;
                }
            }
            if(owner == nullptr) continue;
            OrderType orderType = (type == "BUY" ? OrderType::BUY : OrderType::SELL);
            Order* order = new Order(
                owner,
                orderType,
                stod(price),
                stoi(shares),
                stoi(id),
                stoi(sequence),
                stoi(remaining),
                stoi(active) != 0
            );
            manager->restoreOrder(order);
        }
        file.close();
    }
    // Load Trades
    void loadTrades(OrderManager* manager){
        ifstream file(tradeFilePath);
        if(!file.is_open()) return;
        string line;
        vector<Trade*> loadedTrades;
        while(getline(file, line)){
            stringstream ss(line);
            string id, buyID, sellID;
            string price, quantity, timestamp;
            getline(ss, id, separator);
            getline(ss, buyID, separator);
            getline(ss, sellID, separator);
            getline(ss, price, separator);
            getline(ss, quantity, separator);
            getline(ss, timestamp, separator);
            Order* buyOrder = manager->findOrderInAllOrders(stoi(buyID));
            Order* sellOrder = manager->findOrderInAllOrders(stoi(sellID));
            if(buyOrder == nullptr || sellOrder == nullptr) continue;
            Trade* trade = new Trade(
                buyOrder,
                sellOrder,
                stod(price),
                stoi(quantity),
                stoi(id),
                static_cast<time_t>(stoll(timestamp))
            );
            loadedTrades.push_back(trade);
        }
        file.close();
        for(int i = static_cast<int>(loadedTrades.size()) - 1; i >= 0; i--){
            manager->engine->tradeLog->push_front(loadedTrades[i]);
        }
    }
    // Load All Data
    void loadData(vector<User*>& users, OrderManager* manager) {
        loadUsers(users);
        loadOrders(manager, users);
        loadTrades(manager);
    }
};

// Coordinates Entire Engine
// Called from main()
class StockExchange{
    private:
    OrderManager* manager;
    FileHandler fileHandler;
    public:
    // Constructor
    StockExchange(){ manager = new OrderManager(); }
    // Destructor
    ~StockExchange(){ delete manager; }
    // Stock Exchange Main Menu
    void displayMenu(User* &loggedInUser, vector<User*> &users){
        printf("\n===Welcome %s===\n", loggedInUser->getName().c_str());
        printf("\n===Balance: %f===\n", loggedInUser->getUserBal());
        int ch, shares, searchOID;
        double amount, price;
        bool flag;
        do{
            printf("1. Deposit\n");
            printf("2. Withdraw\n");
            printf("3. Create a Buy Order\n");
            printf("4. Create a Sell Order\n");
            printf("5. Display Open Orders\n");
            printf("6. Display Trade History\n");
            printf("7. Cancel Order\n");
            printf("8. Search Order by ID\n");
            printf("9. Log Out\n");
            printf("Enter Choice: ");
            cin>>ch;
            cin.ignore();
            flag = false;
            switch (ch){
            case 1:
                flag = false;
                do{
                    printf("Enter Amount to Deposit: ");
                    cin>>amount;
                    if(amount > 0){
                        flag = true;
                    } else {
                        printf("Amount must be Positive!\n");
                    }
                }while(!flag);
                cin.ignore();
                try{
                    loggedInUser->userDeposit(amount);
                } catch (runtime_error &e){
                    cout<<"Error Occured: "<<e.what()<<endl;
                }
                printf("%f Deposited!\n", amount);
                saveData(users);
                break;
            case 2:
                flag = false;
                do{
                    printf("Enter Amount to Withdraw: ");
                    cin>>amount;
                    if(amount > 0){
                        flag = true;
                    } else {
                        printf("Amount must be Positive!\n");
                    }
                }while(!flag);
                cin.ignore();
                try{
                    loggedInUser->userWithdraw(amount);
                } catch (runtime_error &e){
                    cout<<"Error Occured: "<<e.what()<<endl;
                }
                printf("%f Withdrawn!\n", amount);
                saveData(users);
                break;
            case 3:
                enterPrice(price);
                enterShares(shares);
                cin.ignore();
                manager->placeBuyOrder(loggedInUser, price, shares);
                saveData(users);
                break;
            case 4:
                enterPrice(price);
                enterShares(shares);
                cin.ignore();
                manager->placeSellOrder(loggedInUser, price, shares);
                saveData(users);
                break;
            case 5:
                displayOpenOrders();
                break;
            case 6:
                displayTradeHistory();
                break;
            case 7: {
                printf("Enter Order ID: ");
                cin>>searchOID;
                cin.ignore();
                Order* searchOrder = manager->findOrder(searchOID);
                if(searchOrder->getUser() == loggedInUser){
                    if(searchOrder != nullptr){
                        searchOrder->displayOrder();
                        manager->cancelOrder(searchOrder);
                        printf("Order Cancelled\n");
                        saveData(users);
                    } else {
                        printf("Order Not Found!\n");
                    }
                }
                break;
            }
            case 8: {
                printf("Enter Order ID: ");
                cin>>searchOID;
                cin.ignore();
                Order* searchOrder = manager->findOrder(searchOID);
                if(searchOrder->getUser() == loggedInUser){                
                    if(searchOrder != nullptr){
                        printf("Order Found!\n");
                        searchOrder->displayOrder();
                    } else {
                        printf("Order Not Found!\n");
                    }
                }
                break;
            }
            case 9:
                printf("Logging Out...\n");
                break;
            default:
                printf("Invalid Option! Try Again\n");
                break;
            }
        }while(ch != 9);
    }
    // Display All Open Orders 
    void displayOpenOrders(){ manager->orderTable->display(); }
    // Display Trade Logs
    void displayTradeHistory(){ manager->engine->tradeLog->display(); }
    // Load & Save Methods
    void loadData(vector<User*> &users){ fileHandler.loadData(users, manager); }
    void saveData(vector<User*> &users){ fileHandler.saveData(users, manager); }
};


// Forward Declarations
void mainMenu(StockExchange &exc, vector<User*> &users);
void createAccount(vector<User*> &users);
User* login(vector<User*> &users);

int main(){
    vector<User*> users;
    {
        StockExchange exchange;
        exchange.loadData(users);
        mainMenu(exchange, users);
        exchange.saveData(users);
    }
    for(User* user : users){ delete user; }
    return 0;
}

// Global Defined Functions

// Main Starting Menu
void mainMenu(StockExchange &exc, vector<User*> &users){
    int ch;
    User* loggedInUser = nullptr;
    do{
        printf("===WELCOME TO STOCK EXCHANGE===\n");
        printf("1. Login\n");
        printf("2. Create Account\n");
        printf("3. Exit\n");
        printf("Enter Choice: ");
        cin>>ch;
        cin.ignore();
        switch(ch){
            case 1:
                loggedInUser = login(users);
                break;
            case 2:
                createAccount(users);
                exc.saveData(users);
                break;
            case 3:
                printf("Exiting...\n");
                break;
        }
    }while(ch != 3 && loggedInUser == nullptr);
    if(ch == 3) return;
    exc.displayMenu(loggedInUser, users);
}
// Register's a User
void createAccount(vector<User*> &users){
    string n, p, cp;
    double bal;
    bool usernameFlag = false;
    do{
    printf("Enter a Unique Username: ");
    getline(cin, n);
    // Verify Username
    if(users.size() == 0){
        for(User* user : users){
            if(toLower(user->getName()) == toLower(n)){
                usernameFlag = true;
            }
        }
    }
    }while(!usernameFlag);
    do{
        printf("Enter New Password: ");
        getline(cin, p);
        printf("Confirm Password: ");
        getline(cin, cp);
        if(cp != p){
            printf("Passwords Do Not Match!\n");
        }
    }while(cp != p);
    printf("Continue without Balance to initialize with 0.00\n");
    do{
        printf("Enter Balance to Deposit: ");
        cin>>bal;
        cin.ignore();
        if(bal < 0){
            printf("\n\nBalance must be Positive! Try Again\n\n");
        }
    }while(bal < 0);
    cin.ignore();
    User* newUser = new User(n, p, bal);
    users.push_back(newUser);
}
// Login a User
User* login(vector<User*> &users){
    string n, p;
    double bal;
    int exitChoice;
    do{
        printf("Enter Username: ");
        getline(cin, n);
        printf("Enter Password: ");
        getline(cin, p);
        for(User* u : users){
            if(toLower(u->getName()) == toLower(n) && u->getPassword() == p){
                printf("Logged In!\n");
                return u;
            }
        }
        printf("Invalid Credentials\n");
        printf("1. Try Again\n");
        printf("2. Exit\n");
        printf("Enter Choice: ");
        cin>>exitChoice;
        cin.ignore();
        switch(exitChoice){
            case 1:
                break;
            case 2:
                return nullptr;
                break;
            default:
                break;
        }
    }while(true);
}