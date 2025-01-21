class TcpClient {
    
    boost::asio::io_context m_context;
    boost::asio::ip::tcp::resolver m_resolver;
    boost::asio::ip::tcp::resolver::results_type m_endpoints;
    const std::string m_server_address;
    const unsigned int m_port_num;

public:
    TcpClient(std::string server_address, unsigned int port_num) : 
    m_context(), 
    m_resolver(m_context), 
    m_server_address(server_address), 
    m_port_num(port_num)
    {}
    ~TcpClient();
    void send(messages::MsgHeader::MsgPointer msg, messages::MsgHeader::MsgHandler msg_response_handler)
    {
        tcp::socket socket(m_context);
        tcp::resolver::results_type endpoints = m_resolver.resolve(m_server_address, std::to_string(m_port_num));
    
        boost::system::error_code err;
        boost::asio::connect(socket, endpoints, err);     
        if (err)
        {
            std::cout << "Error establishing conection to " << m_server_address << ":" << std::to_string(m_port_num) << '\n';
            return;
        }
    
        TcpSession session(std::move(socket));
    
        if(session.send_msg(msg))
        {
            MsgHeader::MsgPointer msg_received = session.wait_for_msg();
            MsgHeader::MsgPointer msg_response = msg_response_handler(msg_received);
    
            if (nullptr != msg_response)
            {
                send(msg_response, msg_response_handler);
            } 
        }
    }
};