#pragma once

#define AS_TYPE(TYPE, FIELD)             (*((TYPE*)(&FIELD)))
#define REFERENCE_FROM_PTR(TYPE, PTR)    (*((TYPE*)PTR))
#define MAKE_NULL_OBJECT(OBJ_TYPE)       (*((OBJ_TYPE*)NULL))
