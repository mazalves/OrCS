#ifndef __DV_DEFINES__
#define __DV_DEFINES__
namespace DV
{
enum DV_ERROR
{
 SUCCESS,
 NOT_ENOUGH_VR,
 PC_NOT_FOUND_IN_VRMT,
 NEW_PARAMETERS_REVECTORIZING,
 VECTORIZE_OPERATION_FORWARD,
 NOT_WRITING,
 REGISTER_GREATER_THAN_MAX,
 NEW_PARAMETERS_NOT_VECTORIZED
};
}
// Constants defined by configuration
extern int32_t VECTORIZATION_SIZE; // 4
extern int32_t NUM_VR; // 32
extern int32_t VRMT_SIZE; // 32 					// Acho que precisa ser maior ou igual ao VR. Deve ser 1 <-> 1
extern int32_t VRMT_ASSOCIATIVITY;
extern int32_t TL_SIZE; // 1]
extern int32_t TL_ASSOCIATIVITY;
extern int32_t FU_VALIDATION_SIZE; // 100
extern int32_t FU_VALIDATION_WAIT_NEXT; // 1

extern int32_t FETCH_BUFFER_VECTORIZED; // 100 	// Tamanho do buffer auxiliar que passa instruções
                                    	 	// vetorizadas para o decode
extern int32_t DECODE_BUFFER_VECTORIZED; // 100  // Tamanho do buffer auxiliar que passa instruções
                                    	   // vetorizadas para o rename

extern uint32_t ROB_VECTORIAL_SIZE; // 100 	// Espaço adicional no ROB para instruções vetoriais
		                               			// Supostamente elas não entram no ROB, mas com esse espaço extra
		                               			// dedicado fica mais fácil gerenciar
extern int32_t VECTORIZATION_ENABLED;

extern int32_t MAX_LOAD_STRIDE; // -1 for unlimited

/*
#define VECTORIZATION_SIZE 4
#define NUM_VR 32
#define VRMT_SIZE 32 // Acho que precisa ser maior ou igual ao VR. Deve ser 1 <-> 1
#define TL_SIZE 32
#define FU_VALIDATION_SIZE 100
#define FU_VALIDATION_WAIT_NEXT 1

#define FETCH_BUFFER_VECTORIZED 100 // Tamanho do buffer auxiliar que passa instruções
                                    // vetorizadas para o decode
#define DECODE_BUFFER_VECTORIZED 100 // Tamanho do buffer auxiliar que passa instruções
                                    // vetorizadas para o rename

#define ROB_VECTORIAL_SIZE 100 // Espaço adicional no ROB para instruções vetoriais
                               // Supostamente elas não entram no ROB, mas com esse espaço extra
                               // dedicado fica mais fácil gerenciar

*/
#endif