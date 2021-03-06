#include "kr_input.h"
#include "kr_input_handle.h"
#include "kr_input_define.h"

struct _kr_input_t
{
    T_KRModule       *ptInputModule;
    T_KRList         *ptInputDefineList;
    time_t           tConstructTime;
};


static int _kr_input_define_new(char *psParamClassName, char *psParamObjectKey, 
        char *psParamObjectString, void *ptParamObject, void *data)
{
    T_KRParamInput *ptParamInput = (T_KRParamInput *)ptParamObject;
    T_KRInput *ptInput = (T_KRInput *)data;
    
    T_KRInputDefine *ptInputDefine = kr_input_define_new(ptParamInput);
    if (ptInputDefine == NULL) {
        KR_LOG(KR_LOGERROR, "kr_input_define_new [%ld] error!", \
            ptParamInput->lInputId);
        return -1;
    }
    
    kr_list_add_sorted(ptInput->ptInputDefineList, ptInputDefine, NULL);
    
    return 0;
}


T_KRInput* kr_input_construct(T_KRParam *ptParam, char *psInputModule)
{
    T_KRInput *ptInput = kr_calloc(sizeof(*ptInput));
    if (ptInput == NULL) {
        KR_LOG(KR_LOGERROR, "kr_calloc ptInput failed!");
        return NULL;
    }
    
    /* load input module */
    ptInput->ptInputModule = kr_module_open(psInputModule, RTLD_LAZY);
    if (ptInput->ptInputModule == NULL) {
        KR_LOG(KR_LOGERROR, "kr_module_open %s failed!", psInputModule);
        kr_input_destruct(ptInput);
        return NULL;
    }
    
    /*alloc input define list*/
    ptInput->ptInputDefineList = kr_list_new();
    kr_list_set_match(ptInput->ptInputDefineList, (KRListMatchFunc )kr_input_define_match);
    kr_list_set_free(ptInput->ptInputDefineList, (KRListFreeFunc )kr_input_define_free);
    
    /*create input list*/
    if (kr_param_object_foreach(ptParam, KR_PARAM_INPUT, 
            _kr_input_define_new, ptInput) != 0) {
        KR_LOG(KR_LOGERROR, "_kr_db_build_table Error!");
        kr_input_destruct(ptInput);
        return NULL;
    }
    
    ptInput->tConstructTime = kr_param_load_time(ptParam);
    
    return ptInput;
}


void kr_input_destruct(T_KRInput* ptInput)
{
    if (ptInput) {
        if (ptInput->ptInputDefineList) {
            kr_list_destroy(ptInput->ptInputDefineList);
        }
        if (ptInput->ptInputModule) {
            kr_module_close(ptInput->ptInputModule);
        }
        kr_free(ptInput);
    }
}


int kr_input_check(T_KRInput* ptInput, T_KRParam *ptParam)
{
    /*check input param*/
    if (ptInput->tConstructTime != kr_param_load_time(ptParam)) {
        KR_LOG(KR_LOGDEBUG, "reload ...[%ld][%ld]", 
                ptInput->tConstructTime, kr_param_load_time(ptParam));

        //TODO:reconstruct ptInput
    }
    return 0;
}


T_KRInputDefine* kr_input_get_define(T_KRInput *ptInput, int iInputId)
{
    T_KRListNode *ptListNode = kr_list_search(ptInput->ptInputDefineList, &iInputId);
    if (ptListNode == NULL) {
        return NULL;
    }
    
    return (T_KRInputDefine *)kr_list_value(ptListNode);
}


int kr_input_handle_process(T_KRInputHandle *ptInputHandle, 
        T_KRInputDefine *ptInputDefine, size_t size, void *buff, 
        T_KRRecord *ptRecord)
{
    void *data = buff;

    if (ptInputHandle->pfInputPre) data=ptInputHandle->pfInputPre(buff);
    for (int fldno=0; fldno<ptInputDefine->iFieldCnt; fldno++) {
        int fldlen = kr_record_get_field_length(ptRecord, fldno);
        void *fldval = kr_record_get_field_value(ptRecord, fldno);

        ptInputHandle->pfInput(fldval, fldno, fldlen, data);
    }
    if (ptInputHandle->pfInputPost) ptInputHandle->pfInputPost(data);

    return 0;
}


T_KRRecord *kr_input_process(T_KRInput *ptInput, T_KRRequest *ptRequest)
{
    char *msgid = ptRequest->msgid;
    int msgsrc = ptRequest->msgsrc;
    char *msgfmt = ptRequest->msgfmt;
    size_t msglen = ptRequest->msglen;
    char *msgbuf = ptRequest->msgbuf;

    T_KRInputDefine *ptInputDefine = kr_input_get_define(ptInput, msgsrc);
    if (ptInputDefine == NULL) {
        KR_LOG(KR_LOGERROR, "kr_input_get_define [%d] error!", msgsrc);
        return NULL;
    }
    
    T_KRInputHandle *ptInputHandle = kr_input_define_get_handle(ptInputDefine, msgfmt, ptInput->ptInputModule);
    if (ptInputHandle == NULL) {
        KR_LOG(KR_LOGERROR, "kr_input_define_get_handle [%s] error!", msgfmt);
        return NULL;
    }
    
    T_KRRecord *ptRecord = kr_record_new(ptInput, msgsrc);
    if (ptRecord == NULL) {
        KR_LOG(KR_LOGERROR, "kr_record_new [%s] error!", msgsrc);
        return NULL;
    }
    
    int iResult = kr_input_handle_process(ptInputHandle, ptInputDefine, msglen, msgbuf, ptRecord);
    if (iResult != 0) {
        KR_LOG(KR_LOGERROR, "kr_input_handle_process [%s] error!", msgid);
        kr_record_free(ptRecord);
        return NULL;
    }
    
    return ptRecord;
}
