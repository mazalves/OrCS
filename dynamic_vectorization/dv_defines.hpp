namespace DV
{
enum DV_ERROR
{
 SUCCESS,
 NOT_ENOUGH_VR,
 PC_NOT_FOUND_IN_VRMT,
 NEW_PARAMETERS_REVECTORIZING,
 VECTORIZE_OPERATION_FORWARD
};
}

#define VECTORIZATION_SIZE 4
#define NUM_VR 10
#define VRMT_SIZE 10 // Acho que precisa ser maior ou igual ao VR. Deve ser 1 <-> 1
#define TL_SIZE 10