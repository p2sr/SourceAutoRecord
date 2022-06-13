#pragma once

class IGameEvent {
public:
	virtual ~IGameEvent() = default;
	virtual const char *GetName() const = 0;
	virtual bool IsReliable() const = 0;
	virtual bool IsLocal() const = 0;
	virtual bool IsEmpty(const char *key = 0) = 0;
	virtual bool GetBool(const char *key = 0, bool default_value = false) = 0;
	virtual int GetInt(const char *key = 0, int default_value = 0) = 0;
	virtual float GetFloat(const char *key = 0, float default_value = 0.0f) = 0;
	virtual const char *GetString(const char *key = 0, const char *default_value = "") = 0;
	virtual void SetBool(const char *key, bool value) = 0;
	virtual void SetInt(const char *key, int value) = 0;
	virtual void SetFloat(const char *key, float value) = 0;
	virtual void SetString(const char *key, const char *value) = 0;
};

class IGameEventListener2 {
public:
	virtual ~IGameEventListener2() = default;
	virtual void FireGameEvent(IGameEvent *event) = 0;
	virtual int GetEventDebugID() = 0;
};
