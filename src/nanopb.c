#include "nanopb.h"

static uint32_t Ack_Done = 0U;

void nanopb_error(data_t *response){
    response->data[0] = 3U;
    response->data_len = 1U;

}

void write_stdio(data_t *buff){
    uint32_t idx;
    stdio_put_string(buff->data, buff->data_len, false, false);
    stdio_flush();

    for(idx = 0U; idx < buff->data_len; idx++){
        buff->data[idx] = 0U;
    }
    buff->data_len = 0U;
    
}


/**
 * Processing Special 1 bytes request
 */
uint8_t special_request(data_t *request, data_t *response){

    Assignment msg = Assignment_init_zero;
    pb_ostream_t stream;
    uint8_t status = 0U;
    if(request->data[0] == 0U){
        msg.asn = DevKind_MotorDriverL;
        status = encode_request_msg(response, &msg);
        Ack_Done  = 0U;
    }
    return status;
}

uint8_t processing_msg_request(data_t *buffer, Assignment *msg){
    uint8_t status;
    switch (msg->asn)
    {
    case DevKind_HostAck:
        msg->asn = DevKind_DevAck;
        status = encode_request_msg(buffer, msg);
        Ack_Done = 1U;
        break;
    
    default:
        nanopb_error(buffer);
        status = 0U;
        break;
    }
    return status;
}

uint8_t encode_request_msg(data_t *buffer, Assignment *msg){
    pb_ostream_t stream;
    stream = pb_ostream_from_buffer(buffer->data, PROTO_PACKET_SIZE);
    bool status = 0U;
    status = pb_encode(&stream, Assignment_fields, msg);

    if(!status)
        return 0U;

    buffer->data_len = stream.bytes_written;

    return 1U;
}


uint8_t decode_request_msg(data_t *buff, Assignment *message){

    /* Function to decoding message from ProtoBuf*/
    bool status;
    pb_istream_t stream = pb_istream_from_buffer(buff->data, buff->data_len);
    status = pb_decode(&stream, Assignment_fields, message);
    if(!status)
        return 0U;

    return 1U;
    
}

/*
Processing request and return message
status: 0 for fail and 1 for sucess
*/
uint8_t process_request(data_t *request, data_t *response){
    
    Assignment msg = Assignment_init_zero;
    uint8_t status = 0U;
    pb_ostream_t stream;

    if(request->data_len == 1U){
        status = special_request(request, response);
        return status;
    }

    if(Ack_Done == 0U){
        status = decode_request_msg(request, &msg);
        if(status == 1U){
            status = processing_msg_request(response, &msg);
            return status;
        }
    } else {

    }

    if(status == 0U){
        nanopb_error(response);
    }



    return status;
}