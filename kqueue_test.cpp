/**
 * kqueue事件监听示例程序
 *
 * 本程序演示了如何使用macOS/BSD系统的kqueue机制来监听多种类型的事件：
 * 1. 标准输入的可读事件（EVFILT_READ）
 * 2. 文件的修改事件（EVFILT_VNODE）
 *
 * kqueue是一个高效的事件通知机制，类似于Linux的epoll，但功能更强大，
 * 可以监听文件描述符、信号、进程、文件系统变化等多种事件类型。
 *
 * 编译命令: g++ -std=c++23 -o kqueue_test kqueue_test.cpp
 * 运行方式: ./kqueue_test
 *
 * 适用于macOS和BSD系统
 */

// 系统头文件包含
#include <cerrno>      // 定义errno全局变量和各种错误码常量（如EINTR, EAGAIN等）
#include <cstring>     // C字符串操作函数（strlen, strcpy, memset, strerror等）
#include <fcntl.h>     // 文件控制选项和标志（O_CREAT, O_RDWR, O_NONBLOCK等）
#include <iostream>    // C++标准输入输出流（std::cout, std::cerr等）
#include <string>      // C++字符串类std::string及其相关操作
#include <sys/event.h> // kqueue相关的结构体和函数声明（kqueue, kevent, struct kevent等）
#include <sys/stat.h>  // 文件状态和权限相关定义（S_IRUSR, S_IWUSR等）
#include <sys/time.h>  // 时间相关结构体和函数（struct timespec, gettimeofday等）
#include <sys/types.h> // 基本数据类型定义（pid_t, size_t, ssize_t, uintptr_t等）
#include <unistd.h>    // UNIX标准函数定义（read, write, close, lseek等）
#include <bitset>

/**
 * 错误处理辅助函数
 *
 * 当系统调用失败时，打印错误信息并退出程序。
 * 使用strerror(errno)将错误码转换为可读的错误描述。
 *
 * @param message 自定义的错误描述信息
 */
void error_exit(const std::string &message)
{
    // 输出错误信息到标准错误流，格式为："错误描述: 系统错误信息"
    std::cerr << message << ": " << strerror(errno) << std::endl;
    // 以失败状态码退出程序
    exit(EXIT_FAILURE);
}

int main()
{
    // ========== 第一步：创建kqueue实例 ==========

    /**
     * 创建一个新的kqueue实例
     *
     * kqueue()系统调用创建一个新的内核事件队列，返回一个文件描述符。
     * 这个文件描述符可以用于后续的kevent()调用来注册事件和等待事件。
     * kqueue本身也是一个文件描述符，可以被select/poll监听，也可以被其他kqueue监听。
     */
    int kq = kqueue();
    if (kq == -1)
    {
        error_exit("创建kqueue失败");
    }

    // 输出程序启动信息
    std::cout << "kqueue示例启动..." << std::endl;
    std::cout << "监听标准输入和测试文件..." << std::endl;

    // ========== 第二步：准备测试文件 ==========

    /**
     * 创建并打开一个测试文件用于监视文件系统事件
     *
     * open()参数说明：
     * - O_CREAT: 如果文件不存在则创建
     * - O_RDWR:  以读写模式打开
     * - S_IRUSR: 文件所有者具有读权限
     * - S_IWUSR: 文件所有者具有写权限
     */
    const char *test_file_path = "kqueue_test_file.txt";
    const int test_file = open(test_file_path, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (test_file == -1)
    {
        error_exit("打开测试文件失败");
    }

    // 向测试文件写入初始内容，以便后续观察文件变化
    const char *initial_content = "这是初始内容\n";
    write(test_file, initial_content, strlen(initial_content));

    // 输出用户操作提示信息
    std::cout << "创建了测试文件: " << test_file_path << std::endl;
    std::cout << "你可以在另一个终端中修改该文件来触发事件" << std::endl;
    std::cout << "例如: echo '新内容' >> " << test_file_path << std::endl;
    std::cout << "或者在此终端输入内容来触发标准输入事件" << std::endl;

    // ========== 第三步：配置要监听的事件 ==========

    /**
     * 创建kevent结构体数组来定义要监听的事件
     *
     * struct kevent结构体字段说明：
     * - ident:   事件标识符（通常是文件描述符）
     * - filter:  事件过滤器类型（如EVFILT_READ, EVFILT_VNODE等）
     * - flags:   事件标志（如EV_ADD添加, EV_DELETE删除, EV_CLEAR清除等）
     * - fflags:  过滤器特定标志（如NOTE_WRITE, NOTE_DELETE等）
     * - data:    过滤器特定数据（如可读字节数）
     * - udata:   用户自定义数据指针
     */
    struct kevent change_events[2] = {
        // 第一个事件：监听标准输入的可读事件
        {
            .ident = STDIN_FILENO, // 标准输入文件描述符（通常为0）
            .filter = EVFILT_READ, // 读事件过滤器：当有数据可读时触发
            .flags = EV_ADD,       // EV_ADD：将此事件添加到kqueue中
            .fflags = 0,           // 对于EVFILT_READ，通常不需要额外标志
            .data = 0,             // 系统填充：可读的字节数
            .udata = NULL          // 用户数据：可以存储自定义指针，这里不使用
        },
        // 第二个事件：监听测试文件的文件系统变化事件
        {
            .ident = static_cast<uintptr_t>(test_file), // 测试文件的文件描述符
            .filter = EVFILT_VNODE,                     // 文件节点事件过滤器：监听文件系统变化
            .flags = EV_ADD | EV_CLEAR,                 // EV_ADD添加事件 | EV_CLEAR事件触发后自动清除
            .fflags = NOTE_WRITE | NOTE_ATTRIB,         // NOTE_WRITE监听写入 | NOTE_ATTRIB监听属性变化
            .data = 0,                                  // 系统填充
            .udata = NULL                               // 用户数据：这里不使用
        }};

    /**
     * 将事件注册到kqueue中
     *
     * kevent()函数参数说明：
     * - kq:           kqueue文件描述符
     * - changelist:   要注册/修改的事件数组
     * - nchanges:     changelist数组的长度
     * - eventlist:    用于接收触发事件的数组（这里为NULL，表示只注册不接收）
     * - nevents:      eventlist数组的长度（这里为0）
     * - timeout:      超时设置（这里为NULL，表示不设置超时）
     *
     * 返回值：成功时返回放入eventlist的事件数量，失败时返回-1
     */
    if (kevent(kq, change_events, 2, NULL, 0, NULL) == -1)
    {
        close(test_file); // 清理已打开的文件描述符
        error_exit("注册事件失败");
    }

    // ========== 第四步：主事件循环 ==========

    /**
     * 准备事件接收缓冲区和数据读取缓冲区
     */
    struct kevent event_data[10]; // 用于接收触发的事件，最多同时处理10个事件
    char *buffer = new char[256]; // 用于读取标准输入数据的缓冲区

    std::cout << "\n开始事件循环，等待事件发生...\n"
              << std::endl;

    // 无限循环等待和处理事件
    while (true)
    {
        /**
         * 等待事件发生
         *
         * 这次kevent()调用用于接收事件：
         * - changelist为NULL：不注册新事件
         * - nchanges为0：不注册新事件
         * - eventlist为event_data：用于接收触发的事件
         * - nevents为10：最多接收10个事件
         * - timeout为NULL：无限等待直到有事件发生
         *
         * 返回值：触发的事件数量，失败时返回-1
         */
        int num_events = kevent(kq, NULL, 0, event_data, 10, NULL);

        // 检查kevent调用是否失败
        if (num_events == -1)
        {
            // EINTR表示系统调用被信号中断，这是正常情况，继续等待即可
            if (errno == EINTR)
            {
                continue; // 被信号中断，继续等待
            }
            // 其他错误则退出程序
            close(test_file);
            error_exit("kevent等待失败");
        }

        // ========== 第五步：处理触发的事件 ==========

        /**
         * 遍历所有触发的事件并进行相应处理
         */
        for (int i = 0; i < num_events; i++)
        {
            // 获取触发事件的文件描述符
            int event_fd = event_data[i].ident; // identifier for this event

            // ===== 处理标准输入事件 =====
            if (event_fd == STDIN_FILENO)
            {
                // 确认这是一个读事件（虽然我们只注册了读事件，但最好还是检查）
                if (event_data[i].filter != EVFILT_READ)
                    continue;

                // 清空缓冲区，确保没有残留数据
                std::fill(buffer, buffer + 256, 0);

                /**
                 * 从标准输入读取数据
                 *
                 * read()函数参数：
                 * - STDIN_FILENO: 标准输入文件描述符
                 * - buffer: 数据缓冲区
                 * - 255: 最多读取255字节（留一个字节给'\0'）
                 *
                 * 返回值：实际读取的字节数，0表示EOF，-1表示错误
                 */
                ssize_t bytes_read = read(STDIN_FILENO, buffer, 255);
                if (bytes_read == -1)
                {
                    perror("read failed");
                }
                else if (bytes_read == 0)
                {
                    std::cout << "EOF reached" << std::endl;
                    continue; // 读取失败或没有数据，继续处理下一个事件
                }

                buffer[bytes_read] = '\0'; // 确保字符串以null结尾
                std::cout << "收到标准输入: " << buffer;

                /**
                 * 检查退出命令
                 * 如果用户输入包含"quit"或"exit"，则优雅地退出程序
                 */
                std::string input(buffer, bytes_read);
                if (input.find("quit") != std::string::npos || input.find("exit") != std::string::npos)
                {
                    std::cout << "退出程序..." << std::endl;
                    // 清理资源
                    delete[] buffer; // 释放动态分配的内存
                    close(test_file);
                    close(kq);
                    return 0;
                }
            }
            // ===== 处理文件变化事件 =====
            else if (event_fd == test_file)
            {
                /**
                 * 确认这是文件节点事件且包含写入标志
                 *
                 * fflags字段包含了具体的文件变化类型：
                 * - NOTE_WRITE: 文件内容被修改
                 * - NOTE_ATTRIB: 文件属性被修改（如权限、时间戳等）
                 * - NOTE_DELETE: 文件被删除
                 * - NOTE_RENAME: 文件被重命名
                 */
                if (event_data[i].filter != EVFILT_VNODE) // filter表示事件源
                    continue;

                // 检查是否是写入事件
                if (event_data[i].fflags & NOTE_WRITE)
                {
                    std::cout << "检测到文件写入事件！" << std::endl;

                    /**
                     * 读取文件的最新内容
                     *
                     * lseek()将文件指针重置到文件开头：
                     * - test_file: 文件描述符
                     * - 0: 偏移量（从开头开始）
                     * - SEEK_SET: 从文件开头计算偏移量
                     */
                    lseek(test_file, 0, SEEK_SET);

                    // char file_buffer[1024]; // 文件内容缓冲区
                    ssize_t file_bytes_read = read(test_file, buffer, sizeof(buffer) - 1);

                    if (file_bytes_read > 0)
                    {
                        buffer[file_bytes_read] = '\0'; // 确保字符串结尾
                        std::cout << "文件当前内容: " << buffer << std::endl;
                    }
                    else
                    {
                        std::cout << "无法读取文件内容或文件为空" << std::endl;
                    }
                }

                // 检查是否是属性变化事件
                if (event_data[i].fflags & NOTE_ATTRIB)
                {
                    std::cout << "检测到文件属性变化事件！" << std::endl;
                }
                // 检查其他可能的文件事件
                if (event_data[i].fflags & NOTE_DELETE)
                {
                    std::cout << "文件被删除！" << std::endl;
                }

                if (event_data[i].fflags & NOTE_RENAME)
                {
                    std::cout << "文件被重命名！" << std::endl;
                }
            }
        }
    }

    // ========== 程序清理阶段 ==========

    /**
     * 清理分配的资源
     *
     * 注意：在正常情况下，由于上面是无限循环，这段代码不会被执行。
     * 但为了代码的完整性和良好的编程习惯，我们还是包含了资源清理代码。
     * 实际的清理工作在退出条件中已经完成。
     */
    delete[] buffer;  // 释放动态分配的内存
    close(test_file); // 关闭测试文件描述符
    close(kq);        // 关闭kqueue文件描述符

    return 0; // 程序正常退出
}