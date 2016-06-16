struct Request {
    1:i32 msg_size;
    2:i32 garbage;
}

struct Response {
    1:string msg;
    2:i32 garbage;
}

service HelloSevice
{
    Response say_hello(1:Request req);
}
