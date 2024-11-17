typedef void (*SUBCallbackFunction)();

class SUB{
public:
    static void clear();
    static void setCallback(SUBCallbackFunction callback);
    static void run();
};