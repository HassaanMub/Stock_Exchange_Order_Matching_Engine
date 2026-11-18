#include <iostream> // Used
#include <string>
#include <cstdlib>
#include <cctype>
#include <ctime>
#include <math.h>
#include <vector>
#include <stdexcept>
#include <algorithm>
//Custom Header Files
#include <utils.h>
using namespace std;

class Account;
class User;
class Order;
class Trade;
class OrderQueue;
class OrderHeap;
class HashTable;
class MatchingEngine;
class OrderManager;
class StockExchange;

enum class OrderType{ BUY, SELL };
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
class User{
    private:
    string name;
    string pass;
    Account account;
    public:
    User(string n = "Guest", string password = "123", double initialBalance = 0.0): name(n), pass(password), account(initialBalance) {}
    string getName(){ return name; }
    string getPassword(){ return pass; }
    double getUserBal() { return account.getBal(); }
    void userDeposit(double amount){ account.deposit(amount); }
    double userWithdraw(double withD){ return account.withdraw(withD); }
};
// Represents an individual order
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
    int getOrderID() const { return orderID; }
    double getPrice() const { return price;}
    int getRemainingShares() const { return remainingShares; }
    bool isActive() const { return active == true; }
    int getSequenceNumber() const { return sequenceNumber; }
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
    // int getShares() const { return shares;} // Not Used Currently (NUC)
};
int Order::idCounter = 100;
int Order::sequenceCounter = 1;
// // Represents an Executed Trade
class Trade{
    private:
    int tradeID;
    int buyOrderID;
    int sellOrderID;
    double executionPrice;
    int tradedShares;
    time_t executionTime;
    static int tradeCounter;
    public:
    Trade(Order* buy,
        Order* sell,
        double price,
        int quantity
    ):  buyOrderID(buy->getOrderID()),
        sellOrderID(sell->getOrderID()),
        tradedShares(quantity),
        executionPrice(price),
        executionTime(time(nullptr)),
        tradeID(tradeCounter++) {}
    // int getTradeID() const { return tradeID; } // (NUC)
    // double getExecutionPrice() const { return executionPrice; } // (NUC)
    // int getTradedShares() const { return tradedShares; } // (NUC)
    void displayTrade() const {
        cout<<"Trade ID: T"<<tradeID<<endl;
        cout<<"Buy Order: B"<<buyOrderID<<endl;
        cout<<"Sell Order: S"<<sellOrderID<<endl;
        cout<<"Execution Price: "<<executionPrice<<endl;
        cout<<"Traded Shares: "<<tradedShares<<endl;
    }
};
int Trade::tradeCounter = 1; 
// // Incoming Order Queue
class OrderQueue{
    private:
    struct Node{
        public:
        Order* order;
        Node* next;
        Node(Order* order): order(order), next(nullptr){}
    };
    Node* front;
    Node* rear;
    public:
    OrderQueue(){ front = rear = nullptr; }
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
    ~OrderQueue(){ while(!isEmpty()){ dequeue(); }}
};
// Buy and Sell Priority Heaps
class OrderHeap{
    private:
        Order** heap;
        int capacity;
        int heapSize;
        OrderType type;
        bool hasHigherPriority(Order* a, Order* b) const {
            if(a->getPrice() != b->getPrice()){
                if(type == OrderType::BUY)
                    return a->getPrice() > b->getPrice();
                return a->getPrice() < b->getPrice();    
            }
            return a->getSequenceNumber() < b->getSequenceNumber();
        }
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
    ~OrderHeap(){
        delete[] heap;
    }
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
// Order ID Lookup
class HashTable {
private:
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
// Matches buy and sell orders
class MatchingEngine{
    private:
    OrderHeap* buyHeap;
    OrderHeap* sellHeap;
    public:
    vector<Trade> tradeLog;
    MatchingEngine(){
        buyHeap = new OrderHeap(OrderType::BUY);
        sellHeap = new OrderHeap(OrderType::SELL);
    }
    ~MatchingEngine(){ 
        delete buyHeap; 
        delete sellHeap;
    }
    void processOrder(Order* order){
        // Check if Active or if 0 Shares
        if(!(order->isActive()) || order->getRemainingShares() <= 0) 
            return; 
        // Check if Order is Buy Order
        if(order->type == OrderType::BUY){
            while(order->getRemainingShares() > 0 && !sellHeap->isEmpty()){
                Order* topSell = sellHeap->getTop();
                // If top is inactive, remove it 
                if(!topSell->isActive()){
                    sellHeap->removeTop();
                    continue;
                }
                // If Price Doesn't Match, Skip the Current Iteration
                if(order->getPrice() < topSell->getPrice())
                    break;
                // Remove Top & Match Orders
                sellHeap->removeTop();
                matchOrders(order, topSell, OrderType::BUY);
                // Reinsert SELL if shares remain
                if ((topSell->getRemainingShares() > 0) && (topSell->isActive()))
                sellHeap->insertOrder(topSell);
            }
            // Keep remaining BUY Shares in Buy Heap
            if(order->getRemainingShares() > 0 && order->isActive())
            buyHeap->insertOrder(order);
        }
        // Check if Order is Sell Order
        else if(order->type == OrderType::SELL){
            while(order->getRemainingShares() > 0 && !(buyHeap->isEmpty())){
                Order* topBuy = buyHeap->getTop();
                // If top is inactive, remove it 
                if(!topBuy->isActive()){
                    buyHeap->removeTop();
                    continue;
                }
                // If Price Doesn't Match, Skip the Current Iteration
                if(order->getPrice() > topBuy->getPrice())
                    break;
                // Remove Top & Match Orders
                buyHeap->removeTop();
                matchOrders(topBuy, order, OrderType::BUY);
                // Reinsert SELL if shares remain
                if ((topBuy->getRemainingShares() > 0) && (topBuy->isActive()))
                    buyHeap->insertOrder(topBuy);
            }
            // Keep Remaining SELL Shares in Sell Heap
            if((order->getRemainingShares() > 0) && (order->isActive()))
                sellHeap->insertOrder(order);
        }
    }
    void matchOrders(Order* buy, Order* sell, OrderType type){
        int shareQuantity = min(buy->getRemainingShares(), sell->getRemainingShares());
        executeTrade(buy, sell, shareQuantity, type);
    }
    void executeTrade(Order* buy, Order* sell, int quantity, OrderType type){
        Trade* newTrade = new Trade(buy, sell, (type == OrderType::BUY ? sell->getPrice() : buy->getPrice()), quantity);
        buy->reduceShares(quantity);
        sell->reduceShares(quantity);
        addTrade(newTrade);
    }
    void addTrade(Trade* trade){ tradeLog.push_back(*(trade)); }
};
// Handles Incoming Orders
class OrderManager{
    private:
    public:
    OrderQueue* incomingOrders;
    HashTable* orderTable;
    MatchingEngine* engine;
    OrderManager(){
        incomingOrders = new OrderQueue();
        orderTable = new HashTable();
        engine = new MatchingEngine();
    }
    ~OrderManager(){ 
        delete incomingOrders; 
        delete orderTable; 
        delete engine;
    }
    void placeBuyOrder(User* user, double price, int shares){
        Order* newOrder = new Order(user, OrderType::BUY, price, shares);
        incomingOrders->enqueue(newOrder);
        orderTable->insert(newOrder);
        processPendingOrders();
    }
    void placeSellOrder(User* user, double price, int shares){
        Order* newOrder = new Order(user, OrderType::SELL, price, shares);
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
        }
    }
    void cancelOrder(Order* order){
        orderTable->remove(order->getOrderID());
        order->cancelOrder();
    }
    Order* findOrder(int orderID){
        return orderTable->search(orderID);
    }
};
// Coordinates Entire Engine
// Called from main()
class StockExchange{
    private:
    OrderManager* manager;
    public:
    StockExchange(){
        manager = new OrderManager();
    }
    ~StockExchange(){
        delete manager;
    }
    void displayMenu(User &loggedInUser){
        printf("\n===Welcome %s===\n", loggedInUser.getName().c_str());
        printf("\n===Balance: %d===\n", loggedInUser.getUserBal());
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
            printf("9. Exit\n");
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
                loggedInUser.userDeposit(amount);
                printf("%f Deposited!\n", amount);
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
                loggedInUser.userWithdraw(amount);
                printf("%f Withdrawn!\n", amount);
                break;
            case 3:
                printf("Enter Price");
                cin>>price;
                printf("Enter Shares");
                cin>>shares;
                cin.ignore();
                manager->placeBuyOrder(&loggedInUser, price, shares);
                break;
            case 4:
                printf("Enter Price");
                cin>>price;
                printf("Enter Shares");
                cin>>shares;
                cin.ignore();
                manager->placeSellOrder(&loggedInUser, price, shares);
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
                if(searchOrder != nullptr){
                    searchOrder->displayOrder();
                    manager->cancelOrder(searchOrder);
                    printf("Order Cancelled\n");
                } else {
                    printf("Order Not Found!\n");
                }
                break;
            }
            case 8:
                printf("Enter Order ID: ");
                cin>>searchOID;
                cin.ignore();
                Order* searchOrder = manager->findOrder(searchOID);
                if(searchOrder != nullptr){
                    printf("Order Found!\n");
                    searchOrder->displayOrder();
                } else {
                    printf("Order Not Found!\n");
                }
                break;
            case 9:
                printf("Exiting...\n");
                break;
            default:
                printf("Invalid Option! Try Again\n");
                break;
            }
        }while(ch != 9);
    }
    // void startExchange();
    void displayOpenOrders(){
        manager->orderTable->display();
    }
    void displayTradeHistory(){
        for(Trade t : manager->engine->tradeLog){
            t.displayTrade();
        }
    }
};

// Forward Declarations
void mainMenu(StockExchange &exc, vector<User> &users);
void createAccount(vector<User> &users);
User* login(vector<User> &users);


int main(){
    vector<User> users;
    StockExchange exchange;
    mainMenu(exchange, users);
}

// Global Defined Functions
void mainMenu(StockExchange &exc, vector<User> &users){
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
                break;
            case 3:
                printf("Exiting...\n");
                break;
        }
    }while(ch != 3 && loggedInUser == nullptr);
    if(ch == 3) return;
    exc.displayMenu(*loggedInUser);
}
void createAccount(vector<User> &users){
    string n, p, cp;
    double bal;
    printf("Enter New Username: ");
    getline(cin, n);
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
        if(bal < 0){
            printf("\n\nBalance must be Positive! Try Again\n\n");
        }
    }while(bal < 0);
    cin.ignore();
    User newUser(n, p, bal);
    users.push_back(newUser);
}
User* login(vector<User> &users){
    string n, p;
    double bal;
    int exitChoice;
    do{
        printf("Enter Username: ");
        getline(cin, n);
        printf("Enter Password: ");
        getline(cin, p);
        for(User u : users){
            if(toLower(u.getName()) == toLower(n) && u.getPassword() == p){
                printf("Logged In!\n");
                return &u;
            }
        }
        printf("Invalid Credentials\n");
        printf("1. Try Again\n");
        printf("2. Exit\n");
        printf("Enter Choice: ");
        cin>>exitChoice;
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