//
// Created by s17kf on 31.05.18.
//

#ifndef MSGRECEIVING_COMMON_H
#define MSGRECEIVING_COMMON_H

#define QUEUE_MODE S_IRWXU

#define CHECK(x) \
    do { \
        if (!(x)) { \
            fprintf(stderr, "%s:%d: ", __func__, __LINE__); \
            perror(#x); \
            /*std::cout<<std::strerror(errno)<<std::endl;*/\
            std::cout<<"# error: "<<errno<<std::endl;\
            exit(-1); \
        } \
    } while (0) \


#endif //MSGRECEIVING_COMMON_H
