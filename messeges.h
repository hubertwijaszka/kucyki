typedef struct {
    int section;
    int lamport_clock;
    int action;
    int value;
    } packet_send_t;

typedef struct {
    int section;
    int lamport_clock;
    int earliest;
} packet_response_t;

union Messeges{
    packet_send_t send;
    packet_response_t response;
};