#include "Server.hpp"

void
irc::Server::try_sending_data( void )
{
    size_t                      end_of_line, last_end_of_line;
    int                         bytes_sent;

    pending_sends_iterator_t data_to_send_it = _pending_sends.begin();
    while (data_to_send_it != _pending_sends.end()) {
        last_end_of_line = end_of_line = 0;

        while ((end_of_line = data_to_send_it->second->find("\r\n", end_of_line)) != data_to_send_it->second->npos) {
            end_of_line += 2;
            last_end_of_line = end_of_line;
        }

        if (last_end_of_line != 0) {
            std::cout << "Server -->  " << data_to_send_it->first << ": " << data_to_send_it->second->data();
            if ((bytes_sent = send(data_to_send_it->first, data_to_send_it->second->data(), last_end_of_line, OS_SENDOPT)) == -1) {
                break ;
            } else if (static_cast<size_t>(bytes_sent) != last_end_of_line) {
                last_end_of_line = bytes_sent;
                break ;
            }
        }

        data_to_send_it->second->erase(0, last_end_of_line);

        if (data_to_send_it->second->empty()) {
            pending_sends_iterator_t tmp = data_to_send_it;
            ++data_to_send_it;
            _pending_sends.erase(tmp);
        } else {
            ++data_to_send_it;
        }
    }
}
