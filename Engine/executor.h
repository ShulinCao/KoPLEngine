#ifndef KOPL_EXECUTOR_H
#define KOPL_EXECUTOR_H

#include "engine.h"


typedef struct {
    std::string                                                             function_name;
    std::string                                                             function_key;
    std::string                                                             function_value;
    std::string                                                             function_op;
    int                                                                     dependencies[2] = {-1, -1};
} Function;


typedef  std::vector<std::shared_ptr<Engine::Entities>>                     EntityResultBuffer;
typedef  std::vector<std::shared_ptr<Engine::EntitiesWithFacts>>            EntityWithFactBuffer;


class Executor {
public:
    Engine &                                                                executor_engine;
    explicit Executor(Engine & engine) : executor_engine(engine) {};
};


#endif //KOPL_EXECUTOR_H
