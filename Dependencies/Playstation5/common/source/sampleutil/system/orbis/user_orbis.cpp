/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2019 Sony Interactive Entertainment Inc.
 * 
 */

#include <scebase_common.h>

#if defined(_SCE_TARGET_OS_ORBIS) && _SCE_TARGET_OS_ORBIS

#include <scebase_common.h>
#include <sampleutil/sampleutil_error.h>
#include <sampleutil/system.h>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/event.h>
#include <kernel.h>
#include <user_service.h>

#pragma comment(lib,"libSceUserService_stub_weak.a")

sce::SampleUtil::System::UserIdManager::UserIdManager(UserIdManagerOption *option)
{
	for (int i=0;i<SCE_SAMPLE_UTIL_MAX_LOGIN_USERS;i++) {
		m_loginUserIdList.userId[i] = SCE_SAMPLE_UTIL_USER_ID_INVALID;
		m_userLoginStatusChangedEvents.joinedUserIdList.userId[i] = SCE_SAMPLE_UTIL_USER_ID_INVALID;
		m_userLoginStatusChangedEvents.leftUserIdList.userId[i] = SCE_SAMPLE_UTIL_USER_ID_INVALID;
	}

	m_initialUserId = SCE_SAMPLE_UTIL_USER_ID_INVALID;
	m_returnValueFromUSGetInitialUserFunction = -1;

	int ret; (void)ret;
	ret = initialize();
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
}

sce::SampleUtil::System::UserIdManager::~UserIdManager()
{
	finalize();
}

int sce::SampleUtil::System::UserIdManager::initialize()
{
	int ret = sceUserServiceInitialize(nullptr);
	if (ret != SCE_OK) {
		return ret;
	}

	ret = update();
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

	for (int i=0;i<SCE_SAMPLE_UTIL_MAX_LOGIN_USERS;i++) {
		m_userLoginStatusChangedEvents.joinedUserIdList.userId[i] = SCE_SAMPLE_UTIL_USER_ID_INVALID;
		m_userLoginStatusChangedEvents.leftUserIdList.userId[i] = SCE_SAMPLE_UTIL_USER_ID_INVALID;
	}

	m_returnValueFromUSGetInitialUserFunction = sceUserServiceGetInitialUser(&m_initialUserId);

	return SCE_OK;
}

int sce::SampleUtil::System::UserIdManager::finalize()
{
	return sceUserServiceTerminate();
}

int sce::SampleUtil::System::UserIdManager::update()
{
	for (int i=0;i<SCE_SAMPLE_UTIL_MAX_LOGIN_USERS;i++) {
		m_userLoginStatusChangedEvents.joinedUserIdList.userId[i] = SCE_SAMPLE_UTIL_USER_ID_INVALID;
		m_userLoginStatusChangedEvents.leftUserIdList.userId[i] = SCE_SAMPLE_UTIL_USER_ID_INVALID;
	}

	SceUserServiceEvent event;
	int ret = SCE_OK;
	int i = 0;

	UserIdList previousLoginUserIdList;
	memcpy(&previousLoginUserIdList, &m_loginUserIdList, sizeof(UserIdList));
	
	while (1) {
		ret = sceUserServiceGetEvent(&event);
		if (ret != SCE_OK) {
			break;
		}

		if (event.eventType == SCE_USER_SERVICE_EVENT_TYPE_LOGIN) {
			// update m_loginUserIdList
			for (i=0;i<SCE_SAMPLE_UTIL_MAX_LOGIN_USERS;i++) {
				if (m_loginUserIdList.userId[i] == SCE_SAMPLE_UTIL_USER_ID_INVALID) {
					m_loginUserIdList.userId[i] = event.userId;
					break;
				}
			}
			SCE_SAMPLE_UTIL_ASSERT(i != SCE_SAMPLE_UTIL_MAX_LOGIN_USERS);

			// update m_userLoginStatusChangedEvents.joinedUserIdList
			for (i=0;i<SCE_SAMPLE_UTIL_MAX_LOGIN_USERS;i++) {
				if (m_userLoginStatusChangedEvents.joinedUserIdList.userId[i] == SCE_SAMPLE_UTIL_USER_ID_INVALID) {
					m_userLoginStatusChangedEvents.joinedUserIdList.userId[i] = event.userId;
					break;
				}
			}
			SCE_SAMPLE_UTIL_ASSERT(i != SCE_SAMPLE_UTIL_MAX_LOGIN_USERS);

		} else if(event.eventType == SCE_USER_SERVICE_EVENT_TYPE_LOGOUT) {
			// update m_loginUserIdList
			for (i=0;i<SCE_SAMPLE_UTIL_MAX_LOGIN_USERS;i++) {
				if (m_loginUserIdList.userId[i] == event.userId) {
					m_loginUserIdList.userId[i] = SCE_SAMPLE_UTIL_USER_ID_INVALID;
					break;
				}
			}
			SCE_SAMPLE_UTIL_ASSERT(i != SCE_SAMPLE_UTIL_MAX_LOGIN_USERS);

			// update m_userLoginStatusChangedEvents.joinedUserIdList
			for (i=0;i<SCE_SAMPLE_UTIL_MAX_LOGIN_USERS;i++) {
				if (m_userLoginStatusChangedEvents.joinedUserIdList.userId[i] == event.userId) {
					m_userLoginStatusChangedEvents.joinedUserIdList.userId[i] = SCE_SAMPLE_UTIL_USER_ID_INVALID;
					break;
				}
			}

			// update m_userLoginStatusChangedEvents.leftUserIdList
			bool existOnleftList = false;
			for (i=0;i<SCE_SAMPLE_UTIL_MAX_LOGIN_USERS;i++) {
				if (m_userLoginStatusChangedEvents.leftUserIdList.userId[i] == event.userId) {
					existOnleftList = true;
					break;
				}
			}

			if (!existOnleftList) {
				bool existOnPreviousLoginList = false;
				for (i=0;i<SCE_SAMPLE_UTIL_MAX_LOGIN_USERS;i++) {
					if (previousLoginUserIdList.userId[i] == event.userId) {
						existOnPreviousLoginList = true;
						break;
					}
				}

				if (existOnPreviousLoginList) {
					for (i=0;i<SCE_SAMPLE_UTIL_MAX_LOGIN_USERS;i++) {
						if (m_userLoginStatusChangedEvents.leftUserIdList.userId[i] == SCE_SAMPLE_UTIL_USER_ID_INVALID) {
							m_userLoginStatusChangedEvents.leftUserIdList.userId[i] = event.userId;
							break;
						}
					}
				}
			}
		} else {
			SCE_SAMPLE_UTIL_ASSERT(0);
		}
	}
	
	if (ret != SCE_USER_SERVICE_ERROR_NO_EVENT) {
		return ret;
	}

	return SCE_OK;
}

int sce::SampleUtil::System::UserIdManager::getInitialUser(UserId *userId)
{
	if (userId == nullptr) {
		return SCE_SAMPLE_UTIL_ERROR_NULL_POINTER;
	}

	*userId = m_initialUserId;
	return m_returnValueFromUSGetInitialUserFunction;
}

int sce::SampleUtil::System::UserIdManager::getUserName(const UserId userId, char *userName, const size_t size)
{
	if (userName == nullptr) {
		return SCE_SAMPLE_UTIL_ERROR_NULL_POINTER;
	}

	int ret = sceUserServiceGetUserName(userId, userName, size);
	return ret;
}

int sce::SampleUtil::System::UserIdManager::getUserColor(const UserId userId, UserColor *userColor)
{
	if (userColor == nullptr) {
		return SCE_SAMPLE_UTIL_ERROR_NULL_POINTER;
	}

	SceUserServiceUserColor userServiceUserColor;
	int ret = sceUserServiceGetUserColor(userId, &userServiceUserColor);
	if (ret == SCE_OK) {
		switch (userServiceUserColor) {
		case SCE_USER_SERVICE_USER_COLOR_BLUE:
			*userColor = kUserColorBlue;
			break;
		case SCE_USER_SERVICE_USER_COLOR_RED:
			*userColor = kUserColorRed;
			break;
		case SCE_USER_SERVICE_USER_COLOR_GREEN:
			*userColor = kUserColorGreen;
			break;
		case SCE_USER_SERVICE_USER_COLOR_PINK:
			*userColor = kUserColorPink;
			break;
		}
	}
	return ret;
}

int sce::SampleUtil::System::UserIdManager::getLoginUserIdList(UserIdList *list)
{
	if (list == nullptr) {
		return SCE_SAMPLE_UTIL_ERROR_NULL_POINTER;
	}

	memcpy(list, &m_loginUserIdList, sizeof(UserIdList));

	return SCE_OK;
}

int sce::SampleUtil::System::UserIdManager::getUserLoginStatusChangedEventsOfLastUpdate(UserLoginStatusChangedEvents *events)
{
	if (events == nullptr) {
		return SCE_SAMPLE_UTIL_ERROR_NULL_POINTER;
	}

	memcpy(events, &m_userLoginStatusChangedEvents, sizeof(UserLoginStatusChangedEvents));

	return SCE_OK;
}

#endif //defined(_SCE_TARGET_OS_ORBIS) && _SCE_TARGET_OS_ORBIS
