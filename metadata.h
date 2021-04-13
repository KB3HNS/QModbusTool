/**
 * \file metadata.h
 * \brief Modbus metadata plugin public interface
 * \copyright
 * 2021 Andrew Buettner (ABi)
 * All rights reserved.
 *
 * \section LICENSE
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * (1) Redistributions of source code must retain the above copyright notice, 
 *     this list of conditions and the following disclaimer.
 * (2) Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 * (3) Neither the name of the <copyright holder> nor the names of its 
 *     contributors may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY 
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND 
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef METADATA_DLL_H
#define METADATA_DLL_H


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>  //  uint8_t and friends

#define DATA_BUFFER_REQUIRED_SIZE 252


/**
 * \fn create_request
 * \brief Create a request instance.
 * @param register_number Register number (IE 40001, or 10003)
 * @param function_code [optional, out] update with the function code
 * @return pointer to data, or ``NULL`` if invalid
 */
typedef void* (*create)(uint16_t /*register_number*/, uint8_t* /*function_code*/);
#define CREATE_SYMBOL "create_request"

/**
 * \fn encode_request
 * \brief Encode outgoing request to databuffer
 * @param request Request instance previously created
 * @param data Data buffer (must be at least 252 bytes long)
 * @return number of bytes written to output data, 0 on error
 */
typedef uint8_t (*encode)(void* /*request*/, uint8_t* /*data*/);
#define ENCODE_SYMBOL "encode_request"

/**
 * \fn decode_response
 * \brief Decode incoming request
 * @param request Request instance previously created
 * @param data response buffer
 * @param data_len response length
 * 
 * @return 0 on success, != 0 on error
 */
typedef int (*decode)(void* /*request*/, const uint8_t* /*data*/, uint8_t /*data_len*/);
#define DECODE_SYMBOL "decode_response"

/**
 * \fn decode_label
 * \brief Decode the label section of a response
 * @param request Request instance previously created
 * @param data (Optional) write null-terminated string to buffer
 * @return Number of bytes that have been (or would be) written to data, 
 *         <0 for error
 */
typedef int32_t (*label)(void* /*request*/, char* /*data*/);
#define LABEL_SYMBOL "decode_label"

/**
 * \fn read_min_max
 * \brief Decode the min/max fields of the response
 *
 * @param request Request instance previously created
 * @param min Update with min value
 * @param max Update with max value
 * 
 * @return 0 if response contains data, != 0 otherwise (min/max not updated)
 */
typedef int (*minmax)(void* /*request*/, int32_t* /*min*/, int32_t* /*max*/);
#define MINMAX_SYMBOL "read_min_max"

/**
 * \fn read_default
 * \brief Decode the default value field of the response
 * @param request Request instance previously created
 * @param dflt Update with default value
 * @param max Update with max value
 * @return 0 if response contains data, != 0 otherwise (dflt not updated)
 */
typedef int (*get_default)(void* /*request*/, int32_t* /*dflt*/);
#define DEFAULT_SYMBOL "read_default"

/**
 * \fn get_encoding
 * \brief Decode the register encoding field of the response
 * @param request Request instance previously created
 * @param data Data buffer from protocol
 * @param data_len Total length of data
 * @return Encoding enumeration as int8_t (<0 for no data)
 */
typedef int8_t (*encoding)(void* /*request*/);
#define ENCODING_SYMBOL "get_encoding"

/**
 * \fn release_request
 * \brief Delete a previously created request and release all resources
 * @param request Request instance previously created
 */
typedef void (*release)(void* /*request*/);
#define RELEASE_SYMBOL "release_request"

#ifdef __cplusplus
}
#endif

#endif
