#include "../myhttp/application.h"

int main(int argc, char** argv){
    myhttp::Application app;
    if(app.init(argc, argv)){
        return app.run();
    }
    return 0;
}