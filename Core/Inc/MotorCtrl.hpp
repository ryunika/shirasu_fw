/*
 * MotorCtrl.h
 *
 *  Created on: Sep 10, 2020
 *      Author: ryu
 */

#ifndef SRC_MOTORCTRL_H_
#define SRC_MOTORCTRL_H_

#include "stm32f3xx_hal.h"

using Float_Type = float;

union E{
	uint32_t ADCDualConvertedValue;
	uint16_t ADCConvertedValue[2];
};

struct DataStruct{
	Float_Type current;
};

class MotorCtrl {
public:
	enum class Mode {duty,current,velocity,position,disable};
private:
	using MemberFunc = void (MotorCtrl::*)(void);

	class PI{
	public:
		Float_Type Kp;
		Float_Type Ki;
		void reset()
		{
			u = 0;
			e_prev = 0;
		}
		Float_Type update(Float_Type e)
		{
			u += Kp*(e - e_prev)+Ki*T*e;
			e_prev = e;
			return u;
		}
	private:
		Float_Type u;
		Float_Type e_prev;

	};

	class Motor{
	public:
		Float_Type R;
		Float_Type L;
		void reset()
		{
			i_prev = 0;
		}
		Float_Type inverse(Float_Type i)
		{
			Float_Type y = (L/T+R)*i - L/T*i_prev;
			i_prev = i;
			return y;
		}
	private:
		Float_Type i_prev;
	};

private:
	TIM_HandleTypeDef* tim_pwm;
	TIM_HandleTypeDef* tim_it;
	ADC_HandleTypeDef* adc_master;
	ADC_HandleTypeDef* adc_slave;
	uint16_t ccr_max;
	Mode mode=Mode::disable;
	Float_Type target=0;
	Float_Type voltage=0;

	Float_Type supply_voltage=20;	//TODO:measure supply voltage
	PI current_controller;
	Motor motor;

	void SetDuty(int d);

	void ControlDisable(){};
	void ControlDuty(){};
	void ControlCurrent();

	void Start();
	void Stop();
public:
	DataStruct data;
	void Init(TIM_HandleTypeDef*,ADC_HandleTypeDef*,ADC_HandleTypeDef*);
	Mode GetMode()const;
	void SetMode(Mode);
	Float_Type GetTarget()const;
	void SetTarget(Float_Type);
	void SetVoltage(Float_Type);
	MemberFunc Control = &MotorCtrl::ControlDisable;
	void invoke(uint16_t* buf);
	static constexpr uint16_t ADC_DATA_SIZE=256;
	static constexpr Float_Type T=0.00022756559374454696;	//TODO:set automatically
	bool monitor = false;
	E adc_buff[ADC_DATA_SIZE*2];
};


#endif /* SRC_MOTORCTRL_H_ */