#include <math.h>
#include <algorithm>
#include <tuple>

#include "../se_sdk3/mp_sdk_audio.h"

SE_DECLARE_INIT_STATIC_FILE(ADSR4);

// 0v = 0.001s, 10V = 10s
float VoltageToTime(float v) { return powf(10.0f, ((v) * 0.4f) - 3.0f); }

using namespace gmpi;

std::tuple<double, double, double> CalculateCurve(const double level_, const double sampleRate, double rate, const double target, const double curveAmmount)
{
	double legalLow{};
	double curveRate{};

	rate = (std::min)(2.0, rate);

	double deltaY = target - level_;

	deltaY = fabs(deltaY);

	const double deltaYMin{ 0.000001f };
	if (deltaY < deltaYMin)
	{
		// jump to next_segment
		legalLow = level_ + 1.0;
		return { legalLow , 0.0 , 0.0 };
	}

	// By using more or less of the curve we control straight/curved mix.
	double timeConstants = 10.0 * fabs(curveAmmount);

	// prevent divide-by-zero;
	const double timeConstantsMin{ 0.001f };
	timeConstants = (std::max)(timeConstants, timeConstantsMin); // Prevent divide-by-zero.

	double deltaT = deltaY * sampleRate * VoltageToTime(rate * 10.f);
	deltaT = (std::max)(deltaT, 1.0); // prevent divide-by-zero.

	double stepSize = timeConstants / deltaT;
	if (curveAmmount > 0.0)
	{
		// first step.
		curveRate = 1.0 - exp(-stepSize);
	}
	else
	{
		// 1 before first step.
		curveRate = exp(stepSize) - 1.0;
	}

	// what level does the curve reach after x timeconstants. (We will scale curve to reach 1.0 at this time)
	double valueAtTime1 = 1.0 - exp(-timeConstants);
	auto CurveTarget = deltaY / valueAtTime1; // level curve 'aims' for.

	CurveTarget -= deltaY; // relative to end-level.
	assert(CurveTarget > 0.0);

	if (target <= level_)
	{
		legalLow = target;
		CurveTarget = -CurveTarget;
	}
	else
	{
		legalLow = 0.0;
	}

	if (curveAmmount >= 0) // normal curve.
	{
		// make target relative to segment end level.
		CurveTarget = target + CurveTarget;
	}
	else // inverted curve.
	{
		curveRate = -curveRate;

		// make target relative to segment start level.
		CurveTarget = level_ - CurveTarget; // no, not instantaneous level, as subject to modulation, and other weirdness.
	}

	return { legalLow, curveRate, CurveTarget };
}

std::tuple<double, double, double> CalculateSteadyState(const double target)
{
	return { target - 1.0, 0.0, 0.0 };
}

class Adsr4 : public MpBase2
{
	BoolInPin pinTrigger;
	BoolInPin pinGate;
	AudioInPin pinAttack;
	AudioInPin pinDecay;
	AudioInPin pinSustain;
	AudioInPin pinRelease;
	AudioInPin pinAttackCurve;
	AudioInPin pinDecayCurve;
	AudioInPin pinReleaseCurve;
	AudioOutPin pinSignalOut;
	FloatInPin pinVoiceReset;

	double level_ = {};
	double curveRate_ = {};
	double target_ = {};
	double CurveTarget_ = {};
	double legalLow = {};
	int cur_segment = -1;

public:
	Adsr4()
	{
		initializePin( pinTrigger );
		initializePin( pinGate );
		initializePin( pinAttack );
		initializePin( pinDecay );
		initializePin( pinSustain );
		initializePin( pinRelease );
		initializePin( pinAttackCurve );
		initializePin( pinDecayCurve );
		initializePin( pinReleaseCurve );
		initializePin( pinSignalOut );
		initializePin( pinVoiceReset );
	}

	void subProcess( int sampleFrames )
	{
		const double peakAdsr4Level = 1.0;
		auto out = getBuffer(pinSignalOut);

		for (int s = 0; s < sampleFrames; ++s)
		{
			level_ += curveRate_ * (CurveTarget_ - level_);
			if (level_ < legalLow || level_ > peakAdsr4Level)
			{
				level_ = target_; // undo overshoot.

				TempBlockPositionSetter x(this, getBlockPosition() + s);
				next_segment();
			}

			*out++ = level_;
		}
	}

	void next_segment() // Called when envelope section ends
	{
		++cur_segment;
		const double endLevel = 0.0;
		const double attackPeakLevel = 1.0;

		switch (cur_segment)
		{
		case 0:
			calcCurve(pinAttack, pinAttackCurve, attackPeakLevel);
			break;

		case 1:
			calcCurve(pinDecay, pinDecayCurve, pinSustain);
			break;

		case 2:
			calcSteadyState(pinSustain);
			break;

		case 3:
			calcCurve(pinRelease, pinReleaseCurve, endLevel);
			break;

		default:
			cur_segment = -1;
			calcSteadyState(endLevel);
			break;
		};
	}

	void calcCurve(double rate, double curveAmmount, double target)
	{
		target_ = target;
		pinSignalOut.setStreaming(true);

		std::tie(legalLow, curveRate_, CurveTarget_) = CalculateCurve(level_, getSampleRate(), rate, target, curveAmmount);
	}

	void calcSteadyState(double target)
	{
		target_ = target;
		pinSignalOut.setStreaming(false);

		std::tie(legalLow, curveRate_, CurveTarget_) = CalculateSteadyState(target);
	}

	void onSetPins(void) override
	{
		if (&Adsr4::subProcess != getSubProcess())
		{
			setSubProcess(&Adsr4::subProcess);
			pinSignalOut.setStreaming(false); // first time.
		}

		// Always handle voice reset first, as it can occur in tandem with trigger. (trigger takes precedence)
		if (pinVoiceReset.isUpdated() && pinVoiceReset == 1.0f) // forcedReset
		{
			// _RPT1(_CRT_WARN, "Envelope:: Hard Reset. %d\n", (int) pinVoiceReset  );
			cur_segment = 3;
			next_segment();
		}

		// Check which pins are updated.
		if( pinTrigger.isUpdated() && pinTrigger.getValue() && pinGate.getValue())
		{
			cur_segment = -1;
			next_segment();
		}

		if( pinGate.isUpdated() && !pinGate.getValue())
		{
			if (cur_segment > -1 && cur_segment < 3)
			{
				cur_segment = 2;
				next_segment();
			}
		}
	}
};

namespace
{
	auto r = Register<Adsr4>::withId(L"SE ADSR4");
}
