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
        int ch, shares;
        double amount, price;
        bool flag;
        do{
            printf("1. Deposit\n");
            printf("2. Withdraw\n");
            printf("3. Create a Buy Order\n");
            printf("4. Create a Sell Order\n");
            printf("5. Exit\n");
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
                printf("Exiting...\n");
                break;
            default:
                printf("Invalid Option! Try Again\n");
                break;
            }
        }while(ch != 5);
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
    ~OrderManager(){ delete incomingOrders, orderTable, engine; }
    void placeBuyOrder(User* user, double price, int shares){
        Order* newOrder = new Order(user, OrderType::BUY, price, shares, 1);
        incomingOrders->enqueue(newOrder);
        orderTable->insert(newOrder);
        processPendingOrders(newOrder);
    }
    void placeSellOrder(User* user, double price, int shares){
        Order* newOrder = new Order(user, OrderType::SELL, price, shares, 1);
        incomingOrders->enqueue(newOrder);
        orderTable->insert(newOrder);
        processPendingOrders(newOrder);
    }
    void processPendingOrders(Order* order){
        engine->processOrder(order);
    }
    bool cancelOrder(Order* order){
        order->cancelOrder();
        orderTable->remove(order->getOrderID());
    }
    Order* findOrder(int orderID){
        orderTable->search(orderID);
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
        buyHeap = new OrderHeap();
        sellHeap = new OrderHeap();
    }
    ~MatchingEngine(){ delete buyHeap, sellHeap; }
    void processOrder(Order* order){
        if(order->isActive()){ return; }
        if(order->type == OrderType::BUY){
            if(!(sellHeap->isEmpty())){
                Order* topSell = sellHeap->getTop();
                if(order->getPrice() >= topSell->getPrice()){
                    sellHeap->removeTop();
                    matchOrders(order, topSell, OrderType::BUY);
                }
            }
        }
        if(order->type == OrderType::SELL){
            if(!(buyHeap->isEmpty())){
                Order* topBuy = buyHeap->getTop();
                if(order->getPrice() <= topBuy->getPrice()){
                    buyHeap->removeTop();
                    matchOrders(topBuy, order, OrderType::SELL);
                }
            }
        }
    }
    void matchOrders(Order* buy, Order* sell, OrderType type){
        int shareQuantity = min(buy->getRemainingShares(), sell->getRemainingShares());
        executeTrade(buy, sell, shareQuantity, type);
    }
    void executeTrade(Order* buy, Order* sell, int quantity, OrderType type){
        Trade* newTrade = new Trade(buy, sell, (type == OrderType::BUY ? sell->getPrice() : buy->getPrice()), quantity);
        addTrade(newTrade);
    }
    void addTrade(Trade* trade){ tradeLog.push_back(*(trade)); }
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
    
    static int idTracker;
    
    public:
    OrderType type;
    Order(User* user, 
        OrderType type,
        double price, 
        int shares, 
        int sequenceNumber
    ):  user(user),
        type(type),
        price(price),
        shares(shares),
        sequenceNumber(sequenceNumber)
        {   
            orderID = idTracker++;
            remainingShares = shares;
            active = true;
        }
    int getOrderID() const { return orderID; }
    double getPrice() const { return price;}
    int getShares() const { return shares;}
    int getRemainingShares() const { return remainingShares; }
    bool isActive() const { return active == true; }
    void reduceShares(int decShares) {
        if(decShares < remainingShares || decShares <= 0){
            remainingShares -= decShares;
        } else {
            throw runtime_error("Reduced Share Amount must not be greater than Remaining Shares!\n");
        }
    }
    void cancelOrder() { active = false; }
    void displayOrder() const {
        cout << "Order ID: "<<(type == OrderType::BUY ? "B" : "S")<<orderID << endl;
        cout << "Type: " << (type == OrderType::BUY ? "BUY" : "SELL") << endl;
        cout << "Price: " << price << endl;
        cout << "Original Shares: " << shares << endl;
        cout << "Remaining Shares: " << remainingShares << endl;
    }
};
int Order::idTracker = 100;
// // Represents an Executed Trade
class Trade{
    private:
    static int tradeID;
    int buyOrderID;
    int sellOrderID;
    double executionPrice;
    int tradedShares;
    int executionTime;
    public:
    Trade(Order* buy,
        Order* sell,
        double price,
        int quantity
    ):  buyOrderID(buy->getOrderID()),
        sellOrderID(sell->getOrderID()),
        tradedShares(quantity),
        executionPrice(price),
        executionTime(time(nullptr)) 
        {

        }
    int getTradeID() const { return tradeID; }
    double getExecutionPrice() const { return executionPrice; }
    int getTradedShares() const { return tradedShares; }
    void displayTrade() const {
        cout<<"Trade ID: T"<<tradeID<<endl;
        cout<<"Buy Order: "<<buyOrderID<<endl;
        cout<<"Sell Order: "<<sellOrderID<<endl;
        cout<<"Execution Price: "<<executionPrice<<endl;
        cout<<"Traded Shares: "<<tradedShares<<endl;
    }
};
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
        if(isEmpty()){
            throw runtime_error("OrderQueue is Empty!\n");
        }
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
// // Buy and Sell Priority Heaps
class OrderHeap{
    private:
    Order** heap;
    int capacity;
    int heapSize;
    OrderType type;
    public:
    //constructor
    //destructor
    void insertOrder(Order* order);
    Order* removeTop();
    Order* getTop() const;
    void heapifyUp(int index);
    void heapifyDown(int index);
    bool isEmpty() const { return heapSize == 0; }
    int getSize() const { return heapSize; }
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
    HashTable();
    void insert(Order* order){
        int index = hashFunction(order->getOrderID());
        Node* newNode = table[index];
        newNode->next = table[index];
        table[index] = newNode;
    }
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
    bool remove(int orderID){ // remove the hash table node only
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

void mainMenu(StockExchange &exc, vector<User> &users);
void createAccount(vector<User> &users);
User login(vector<User> &users);


int main(){
    vector<User> users;
    StockExchange exchange;
    mainMenu(exchange, users);
}


void mainMenu(StockExchange &exc, vector<User> &users){
    int ch;
    User loggedInUser = nullptr;
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
    }while(ch != 3 || &loggedInUser != nullptr);
    if(ch == 3) return;
    exc.displayMenu(loggedInUser);
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
    printf("Enter Balance to Deposit: ");
    cin>>bal;
    cin.ignore();
    User newUser(n, p, bal);
    users.push_back(newUser);
}
User login(vector<User> &users){
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
                return u;
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