#include "COSEM.h"
#include "LogicalDevice.h"

namespace EPRI
{
    //
    // AssociationContext
    //
    AssociationContext::AssociationContext(const APPOpenConfirmOrResponse& Response) : 
        m_ClientSAP(Response.m_DestinationAddress), m_ServerSAP(Response.m_SourceAddress), 
        m_SecurityOptions(Response.m_SecurityOptions), m_xDLMS(Response.m_xDLMS),
        m_Status(AssociationStatusType::non_associated)
    {
    }
    //
    //	0	0	40	0	0	    255	Current association	             12	15		Class 12 is SN referencing and Class 15 is LN referencing.
    //	0	0	40	0	1	    255	Association instance 1	         12	15		Idem
    //	0	0	40	0	2 - 127	255	Association instance 2 - 127	 12	15		Idem
    //
    // Association
    //
    Association::Association() :
        IAssociationLNObject({0, 0, 40, 0, {0, 1}, 255})
    {
    }
 
    Association::~Association()
    {
    }

    size_t Association::AvailableAssociations() const
    {
        //
        // TODO - Only 1 association allowed for this release
        //
        return 1 - m_Associations.size();
    }
    
    bool Association::RegisterAssociation(const APPOpenConfirmOrResponse& Response)
    {
        if (AvailableAssociations())
        {
            m_Associations.emplace_back(Response);
            return true;
        }
        return false;
    }
    
    bool Association::ReleaseAssociation(const APPReleaseConfirmOrResponse& Response)
    {
        m_Associations.remove_if(
            [&Response](const AssociationContext& Entry) -> bool
            {
                return Response.m_DestinationAddress == Entry.m_ClientSAP;
            });
        return true;
    }

    const AssociationContext * Association::GetAssociationContext(
        const APPBaseCallbackParameter& Parameter) 
    {
        return GetAssociationContextByAddress(Parameter.m_DestinationAddress);
    }
        
    APDUConstants::Data_Access_Result Association::InternalGet(const AssociationContext& Context,
        ICOSEMAttribute * pAttribute, 
        const Cosem_Attribute_Descriptor& Descriptor, 
        SelectiveAccess * pSelectiveAccess)
    {
        bool                              AppendResult = false;
        uint8_t                           AssociationIndex = 
                                                Descriptor.instance_id.GetValueGroup(COSEMObjectInstanceID::VALUE_GROUP_E);
        AssociationContext *              pContext;
        APDUConstants::Data_Access_Result RetVal = APDUConstants::Data_Access_Result::temporary_failure;
        //
        // Current Association?
        //
        if (0 == AssociationIndex)
        {
            pContext = GetAssociationContextByAddress(Context.m_ClientSAP);
        }
        else
        {
            pContext = GetAssociationContextByIndex(AssociationIndex);
        }
        if (!pContext)
        {
            return APDUConstants::Data_Access_Result::object_unavailable;
        }
        switch (pAttribute->AttributeID)
        {
        case ATTR_OBJ_LIST:
            break;
        case ATTR_PARTNERS_ID:
            AppendResult = pAttribute->Append(
                DLMSSequence
                ({
                    pContext->m_ClientSAP,
                    pContext->m_ServerSAP,
                }));
            if (AppendResult)
            {
                RetVal = APDUConstants::Data_Access_Result::success;
            }
            break;
        case ATTR_APP_CON_NAME:   
            AppendResult = pAttribute->Append(pContext->m_SecurityOptions.ApplicationContextName);
            if (AppendResult)
            {
                RetVal = APDUConstants::Data_Access_Result::success;
            }
            break;
        case ATTR_XDLMS_CON_INFO:     
            AppendResult = pAttribute->Append(
                DLMSSequence
                ({
                    pContext->m_xDLMS.ConformanceBits(),
                    pContext->m_xDLMS.APDUSize(),
                    pContext->m_xDLMS.APDUSize(),
                    pContext->m_xDLMS.DLMSVersion(),
                    pContext->m_xDLMS.QOS(),
                    pContext->m_xDLMS.DedicatedKey()
                }));
            if (AppendResult)
            {
                RetVal = APDUConstants::Data_Access_Result::success;
            }
            break;
        case ATTR_AUTH_MECH_NAME:    
            AppendResult = pAttribute->Append(pContext->m_SecurityOptions.MechanismName);
            if (AppendResult)
            {
                RetVal = APDUConstants::Data_Access_Result::success;
            }
            break;
        case ATTR_SECRET:
            //
            // Not cool to have the SECRET out
            //
            RetVal = APDUConstants::Data_Access_Result::read_write_denied;
            break;
        case ATTR_STATUS:
            AppendResult = pAttribute->Append((uint8_t) pContext->m_Status);
            if (AppendResult)
            {
                RetVal = APDUConstants::Data_Access_Result::success;
            }
            break;
        case ATTR_SECURITY_SETUP_REF:
            //
            // Not cool to have the SECRET out
            //
            RetVal = APDUConstants::Data_Access_Result::read_write_denied;
            break;
        }
        if (AppendResult)
        {
            RetVal = APDUConstants::Data_Access_Result::success;
        }
        return RetVal;
    }
    
    AssociationContext * Association::GetAssociationContextByIndex(int Index)
    {
        AssociationInfoList::iterator it = m_Associations.begin();
        while (it != m_Associations.end())
        {
            if (0 == Index--)
            {
                return &(*it);
            }
            ++it;
        }
        return nullptr;
    }

    AssociationContext * Association::GetAssociationContextByAddress(COSEMAddressType Address)
    {
        AssociationInfoList::iterator it = 
            std::find_if(m_Associations.begin(),
            m_Associations.end(),
            [Address](const AssociationContext& Entry) -> bool
            {
                return Address == Entry.m_ClientSAP;
            });
        if (it == m_Associations.end())
        {
            return nullptr;
        }
        return &(*it);
    }
    //
    // LogicalDevice
    //
    LogicalDevice::LogicalDevice(COSEMServer * pServer) :
        m_pServer(pServer)
    {
        LOGICAL_DEVICE_BEGIN_OBJECTS
            LOGICAL_DEVICE_OBJECT(m_Association)
        LOGICAL_DEVICE_END_OBJECTS
    }
    
    LogicalDevice::~LogicalDevice()
    {
    }
    
    bool LogicalDevice::InitiateOpen(const APPOpenRequestOrIndication& Request, bool UpperLayerAllowed)
    {
        APPOpenConfirmOrResponse::AssociationResultType Result = APPOpenConfirmOrResponse::AssociationResultType::accepted;
        if (!UpperLayerAllowed || m_Association.AvailableAssociations() == 0)
        {
            Result = APPOpenConfirmOrResponse::AssociationResultType::rejected_transient;
        }
        APPOpenConfirmOrResponse 
            Response(SAP(),
                     Request.m_SourceAddress,
                     xDLMS::InitiateResponse(Request.m_xDLMS),
                     Request.m_SecurityOptions,
                     Result,
                     APPOpenConfirmOrResponse::DiagnosticSourceType::acse_service_user,
                     APPOpenConfirmOrResponse::UserDiagnosticType::user_null);
        if (m_pServer->OpenResponse(Response))
        {
            return m_Association.RegisterAssociation(Response);
        }
        return false;
    }

    bool LogicalDevice::InitiateGet(const APPGetRequestOrIndication& Request, bool UpperLayerAllowed)
    {
        if (IsForMe(Request))
        {
            const AssociationContext * pContext = m_Association.GetAssociationContext(Request);
            InvokeIdAndPriorityType    InvokeID = COSEM_GET_INVOKE_ID(Request.m_InvokeIDAndPriority);
            switch (Request.m_Type)
            {
            case APPGetRequestOrIndication::GetRequestType::get_request_normal:
                {
                    DLMSVector                        Data;
                    APDUConstants::Data_Access_Result Result = 
                                                        APDUConstants::Data_Access_Result::scope_of_access_violated;
                    if (pContext)
                    {
                        ssize_t                       ObjectIndex = 
                                                         FindObject(Request.m_Parameter.get<Cosem_Attribute_Descriptor>());
                        if (ObjectIndex != -1)
                        {
                            Result = 
                                m_Objects[ObjectIndex]->Get(*pContext,
                                                             &Data, 
                                                             Request.m_Parameter.get<Cosem_Attribute_Descriptor>());
                        }
                        else
                        {
                            Result = EPRI::APDUConstants::Data_Access_Result::object_unavailable;
                        }
                    }
                    //
                    // Get the data for this attribute and object
                    //
                    return m_pServer->GetResponse(
                        APPGetConfirmOrResponse(SAP(),
                                                Request.m_SourceAddress,
                                                Request.m_InvokeIDAndPriority,
                                                (Result == APDUConstants::Data_Access_Result::success ? 
                                                    Get_Data_Result(Data) : 
                                                    Result)));
                }
                break;
                    
            default:
                throw std::logic_error("InitiateGet Type Not Implemented!");
            }
        }
        return false;
    }
    
    bool LogicalDevice::InitiateSet(const APPSetRequestOrIndication& Request, bool UpperLayerAllowed)
    {
        if (IsForMe(Request))
        {
            const AssociationContext * pContext = m_Association.GetAssociationContext(Request);
            InvokeIdAndPriorityType    InvokeID = COSEM_GET_INVOKE_ID(Request.m_InvokeIDAndPriority);
            switch (Request.m_Type)
            {
            case APPSetRequestOrIndication::SetRequestType::set_request_normal:
                {
                    APDUConstants::Data_Access_Result Result = 
                                                        APDUConstants::Data_Access_Result::scope_of_access_violated;
                    if (pContext)
                    {
                        ssize_t ObjectIndex = 
                            FindObject(Request.m_Parameter.get<Cosem_Attribute_Descriptor>());
                        if (ObjectIndex != -1)
                        {
                            Result = m_Objects[ObjectIndex]->Set(*pContext,
                                                                 Request.m_Parameter.get<Cosem_Attribute_Descriptor>(), 
                                                                 Request.m_Value);
                        }
                        else
                        {
                            Result = EPRI::APDUConstants::Data_Access_Result::object_unavailable;
                        }
                        
                    }
                    return m_pServer->SetResponse(
                                        APPSetConfirmOrResponse(SAP(),
                                            Request.m_SourceAddress,
                                            Request.m_InvokeIDAndPriority,
                                            Result));
                    
                }
                break;

            default:
                throw std::logic_error("InitiateSet Type Not Implemented!");

            }
        }
        return false;
    }
    
    bool LogicalDevice::InitiateAction(const APPActionRequestOrIndication& Request, bool UpperLayerAllowed)
    {
        if (IsForMe(Request))
        {
            const AssociationContext * pContext = m_Association.GetAssociationContext(Request);
            InvokeIdAndPriorityType    InvokeID = COSEM_GET_INVOKE_ID(Request.m_InvokeIDAndPriority);
            switch (Request.m_Type)
            {
            case APPActionRequestOrIndication::ActionRequestType::action_request_normal:
                {
                    APDUConstants::Action_Result Result = 
                        APDUConstants::Action_Result::scope_of_access_violated;
                    if (pContext)
                    {
                        ssize_t ObjectIndex = 
                            FindObject(Request.m_Parameter.get<Cosem_Method_Descriptor>());
                        if (ObjectIndex != -1)
                        {
                            Result = m_Objects[ObjectIndex]->Action(*pContext,
                                                                    Request.m_Parameter.get<Cosem_Method_Descriptor>(), 
                                                                    Request.m_ActionParameters);
                        }
                        else
                        {
                            Result = EPRI::APDUConstants::Action_Result::object_unavailable;
                        }
                    }
                    return m_pServer->ActionResponse(APPActionConfirmOrResponse(SAP(),
                                                        Request.m_SourceAddress,
                                                        Request.m_InvokeIDAndPriority,
                                                        Result));
                }
                break;
                    
            default:
                throw std::logic_error("InitiateAction Type Not Implemented!");

            }
        }
        return false;
    }    
    
    bool LogicalDevice::InitiateRelease(const APPReleaseRequestOrIndication& Request, bool UpperLayerAllowed)
    {
        APPReleaseConfirmOrResponse Response(SAP(),
            Request.m_SourceAddress,
            Request.m_UseRLRQRLRE,
            UpperLayerAllowed ? APPReleaseConfirmOrResponse::ReleaseReason::normal : 
                  APPReleaseConfirmOrResponse::ReleaseReason::not_finished);
        if (m_pServer->ReleaseResponse(Response))
        {
            return m_Association.ReleaseAssociation(Response);
        }
        return false;
    }

    COSEMAddressType LogicalDevice::SAP() const
    {
        return dynamic_cast<COSEM *>(m_pServer)->GetAddress();
    }

    void LogicalDevice::RegisterObject(ICOSEMObject * pObject)
    {
        m_Objects.push_back(pObject);
    }
    
    ssize_t LogicalDevice::FindObject(const Cosem_Attribute_Descriptor& Descriptor) const
    {
        ssize_t RetVal = 0;
        for (; RetVal < m_Objects.size(); ++RetVal)
        {
            if (m_Objects[RetVal]->Supports(Descriptor))
            {
                return RetVal;
            }
        }
        return -1;
    }
    
    ssize_t LogicalDevice::FindObject(const Cosem_Method_Descriptor& Descriptor) const
    {
        ssize_t RetVal = 0;
        for (; RetVal < m_Objects.size(); ++RetVal)
        {
            if (m_Objects[RetVal]->Supports(Descriptor))
            {
                return RetVal;
            }
        }
        return -1;
    }
    
    bool LogicalDevice::IsForMe(const APPBaseCallbackParameter& Parameter) const
    {
        return SAP() == Parameter.m_DestinationAddress ||
            Parameter.m_DestinationAddress == ReservedAddresses::BROADCAST;
    }

}