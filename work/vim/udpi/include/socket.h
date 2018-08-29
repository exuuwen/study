#ifndef _UDPI_SOCKET_H_
#define _UDPI_SOCKET_H_

#ifdef __cplusplus
extern "C" {
#endif

#define IPORT 7501
#define PPORT 8501

void udpi_init_socket(void);
void udpi_socket_dumper_create(uint32_t dumper_id);
void udpi_socket_packet_create(uint32_t packet_id);
void udpi_socket_dumper_write(uint32_t dumper_id, uint8_t *data, size_t len);
void udpi_socket_dumper_read(uint32_t dumper_id);
int udpi_socket_packet_write(uint32_t packet_id, uint8_t *data, size_t len);
void udpi_socket_packet_read(uint32_t packet_id);

#ifdef __cplusplus
}
#endif

#endif
