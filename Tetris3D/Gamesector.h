#ifndef GAMESECTOR_H
#define GAMESECTOR_H


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Block.h"

//Diese Klasse speichert die Weltposition und ob diese Position mit einem Block besetzt ist. 
class GameSector {       
public:             
    
    glm::vec3 WorldPosition;
    glm::mat4 model = glm::mat4(1.0f);
    bool blocked;
    Block block;

    GameSector(){}
     
    GameSector(glm::vec3 wp) {
        WorldPosition = wp;
    }

    GameSector(glm::vec3 wp, Block bl) {
        WorldPosition = wp;
        block = bl;
    }
    
};
#endif