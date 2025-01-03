#ifndef TMC2209_STM32_H
#define TMC2209_STM32_H

#include "stm32f4xx_hal.h"

// 速度脉冲间隔
#define SPEED_STEP_DIVISION 10

// 数据报
#define SYNC  0b101
#define RW_READ  0
#define RW_WRITE  1
#define BITS_PER_BYTE 8
#define BYTE_MAX_VALUE 0xFF
#define DATA_SIZE 4
#define WRITE_READ_REPLY_DATAGRAM_SIZE 8
typedef union WriteReadReplyDatagram {
	struct {
		uint64_t sync :4;              // 位 0-3
		uint64_t reserved :4;          // 位 4-7
		uint64_t serial_address :8;    // 位 8-15
		uint64_t register_address :7;  // 位 16-22
		uint64_t rw :1;                // 位 23
		uint64_t data :32;             // 位 24-55
		uint64_t crc :8;               // 位 56-63
	};
	uint64_t bytes;
} WriteReadReplyDatagram;

// 寄存器：GCONF
#define ADDRESS_GCONF 0x00
typedef union RegisterGlobalConfig {
	struct {
		uint32_t i_scale_analog :1;      // 位 0
		uint32_t internal_rsense :1;     // 位 1
		uint32_t enable_spread_cycle :1; // 位 2
		uint32_t shaft :1;               // 位 3
		uint32_t index_otpw :1;          // 位 4
		uint32_t index_step :1;          // 位 5
		uint32_t pdn_disable :1;         // 位 6
		uint32_t mstep_reg_select :1;    // 位 7
		uint32_t multistep_filt :1;      // 位 8
		uint32_t test_mode :1;           // 位 9
		uint32_t reserved :22;           // 保留位
	};
	uint32_t bytes;
} RegisterGlobalConfig;
extern RegisterGlobalConfig register_gconf;
void setRegisterGCONF(uint8_t uart_instance);

// 寄存器IHOLD_IRUN
#define ADDRESS_IHOLD_IRUN 0x10
typedef union RegisterCurrent {
	struct {
		uint32_t ihold :5;          // 位 0-4: ihold (保持电流)
		uint32_t reserved_0 :3;     // 位 5-7: 保留位
		uint32_t irun :5;           // 位 8-12: irun (运行电流)
		uint32_t reserved_1 :3;     // 位 13-15: 保留位
		uint32_t iholddelay :4;     // 位 16-19: iholddelay (保持延迟)
		uint32_t reserved_2 :12;    // 位 20-31: 保留位
	};
	uint32_t bytes;
} RegisterCurrent;
extern RegisterCurrent register_iholdirun;
void setRegisterIHOLDIRUN(uint8_t uart_instance);

// 寄存器COOLCONF
#define ADDRESS_COOLCONF 0x42
typedef union RegisterCoolConfig {
	struct {
		uint32_t semin :4;          // 位 0-3: semin (最小速度阈值)
		uint32_t reserved_0 :1;     // 位 4: 保留位
		uint32_t seup :2;           // 位 5-6: seup (上升时间)
		uint32_t reserved_1 :1;     // 位 7: 保留位
		uint32_t semax :4;          // 位 8-11: semax (最大速度阈值)
		uint32_t reserved_2 :1;     // 位 12: 保留位
		uint32_t sedn :2;           // 位 13-14: sedn (下降时间)
		uint32_t seimin :1;         // 位 15: seimin (最小电流使能)
		uint32_t reserved_3 :16;    // 位 16-31: 保留位
	};
	uint32_t bytes;                  // 整个寄存器作为一个32位无符号整数
} RegisterCoolConfig;
extern RegisterCoolConfig register_coolconf;
void setRegisterCOOLCONF(uint8_t uart_instance);

// 寄存器 CHOPCONF
#define ADDRESS_CHOPCONF 0x6C
#define MRES_256 0b0000
#define MRES_128 0b0001
#define MRES_064 0b0010
#define MRES_032 0b0011
#define MRES_016 0b0100
#define MRES_008 0b0101
#define MRES_004 0b0110
#define MRES_002 0b0111
#define MRES_001 0b1000
typedef union RegisterChopperConfig {
	struct {
		uint32_t toff :4;           // 位 0-3: toff (关断时间)
		uint32_t hstart :3;         // 位 4-6: hstart (高启动值)
		uint32_t hend :4;           // 位 7-10: hend (高结束值)
		uint32_t reserved_0 :4;     // 位 11-14: 保留位
		uint32_t tbl :2;            // 位 15-16: tbl (空白时间)
		uint32_t vsense :1;         // 位 17: vsense (电流检测模式)
		uint32_t reserved_1 :6;     // 位 18-23: 保留位
		uint32_t mres :4;           // 位 24-27: mres (微步分辨率)
		uint32_t interpolation :1;  // 位 28: 插值模式
		uint32_t double_edge :1;    // 位 29: 双边沿步进
		uint32_t diss2g :1;         // 位 30: 禁用 SG 比较器
		uint32_t diss2vs :1;        // 位 31: 禁用 VSENSE 比较器
	};
	uint32_t bytes;                  // 整个寄存器作为一个32位无符号整数
} RegisterChopperConfig;
extern RegisterChopperConfig register_chopconf;
void setRegisterCHOPCONF(uint8_t uart_instance);


// 寄存器TCOOLTHRS
#define ADDRESS_TCOOLTHRS 0x14
typedef union RegisterTCOOLTHRS {
    struct {
        uint32_t threshold : 20; // 位 0-19: 下限速度阈值
        uint32_t reserved : 12;  // 保留位
    };
    uint32_t bytes;
} RegisterTCOOLTHRS;
extern RegisterTCOOLTHRS register_tcoolthrs;
void setRegisterTCOOLTHRS(uint8_t uart_instance);

// 寄存器SGTHRS
#define ADDRESS_SGTHRS 0x40
typedef union RegisterSGTHRS {
    struct {
        uint8_t threshold : 8; // 位 0-7: 检测阈值
    };
    uint8_t bytes;
} RegisterSGTHRS;
extern RegisterSGTHRS register_sgthrs;
void setRegisterSGTHRS(uint8_t uart_instance);


// 反转数据位
uint32_t reverseData(uint32_t data);
// 计算crc
uint8_t calculateCrc(unsigned char *datagram, size_t length);

// 写寄存器
void writeRegister(uint8_t uart_instance, uint8_t register_address, uint32_t data);


// 定义单个电机动作
typedef struct {
	uint32_t targetSteps;     // 目标步数
	uint32_t currentSteps;    // 当前已完成步数
	uint32_t speed;              // 转速 (0-100)
	uint32_t timsCounter;	// 中断总数
	int8_t direction;         // 方向：1为正转，-1为反转
	uint8_t isActive;         // 是否正在运行
} MotorAction;

// 定义电机引脚
typedef struct {
	GPIO_TypeDef *port_en;    // 使能端口
	uint16_t pin_en;          // 使能引脚
	GPIO_TypeDef *port_dir;   // 方向端口
	uint16_t pin_dir;         // 方向引脚
	GPIO_TypeDef *port_step;  // 脉冲端口
	uint16_t pin_step;        // 脉冲引脚
	GPIO_TypeDef *port_uart;  // 串口端口
	uint16_t pin_uart;        // 串口引脚
	GPIO_TypeDef *port_diag;  // diag端口
	uint16_t pin_diag;        // diag引脚
} DriverPins;

// 电机数量
//#define NUM_MOTORS 6
#define NUM_MOTORS 2

// 定义电机控制结构体
typedef struct {
    uint8_t motorID;          // 电机 ID
    MotorAction action;       // 电机动作状态
    DriverPins pins;          // 电机引脚配置
} StepperMotorControl;

//typedef struct {
//	uint32_t targetSteps;     // 目标步数
//	uint32_t currentSteps;    // 当前已完成步数
//	uint32_t speed;              // 转速 (0-100)
//	uint32_t timsCounter;	// 中断总数
//	int8_t direction;         // 方向：1为正转，-1为反转
//	uint8_t isActive;         // 是否正在运行
//	// GPIO
//	GPIO_TypeDef *port_en;    // 使能端口
//	uint16_t pin_en;          // 使能引脚
//	GPIO_TypeDef *port_dir;   // 方向端口
//	uint16_t pin_dir;         // 方向引脚
//	GPIO_TypeDef *port_step;  // 脉冲端口
//	uint16_t pin_step;        // 脉冲引脚
//	GPIO_TypeDef *port_uart;  // 串口端口
//	uint16_t pin_uart;        // 串口引脚
//	uint16_t uart_index;  // 串口编号
//} StepperMotorControl;

// 电机驱动基本参数
extern StepperMotorControl motors[NUM_MOTORS];

// 电机动作队列

// 初始化电机定义
void motorsDefine(void);
// 初始化电机驱动
void motorsDriverSetup(void);

// 电机运动控制
void motorsActionQueuePush(uint8_t motorIndex, uint32_t steps, uint32_t speed, int8_t direction);

// 电机运动控制定时器中断回调
void motorsStepHandler(void);

// 软件串口发送消息
void TMC2209_sendRawMessage(uint8_t instance, const uint8_t *message, size_t length);




#endif // TMC2209_STM32_H
