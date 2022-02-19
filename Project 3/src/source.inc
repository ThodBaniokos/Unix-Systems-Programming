TRV_M_C_OBJS += $(MDLS)/trv_mon_client/ioAPI/ioAPI.o
TRV_M_C_OBJS += $(MDLS)/trv_mon_client/ioAPI/commands.o
TRV_M_C_OBJS += $(MDLS)/trv_mon_client/helpers/travelMonitorClientHelpers.o

MON_S_OBJS += $(MDLS)/mon_server/cb/cb.o
MON_S_OBJS += $(MDLS)/mon_server/tools/answer.o
MON_S_OBJS += $(MDLS)/mon_server/tools/general.o
MON_S_OBJS += $(MDLS)/mon_server/skip/skipList.o
MON_S_OBJS += $(MDLS)/mon_server/person/person.o
MON_S_OBJS += $(MDLS)/mon_server/ioAPI/mon_server_commands.o
MON_S_OBJS += $(MDLS)/mon_server/helpers/monitorServerHelpers.o

C_OBJS += $(MDLS)/common_impls/ipc_protocol/ipc_protocol.o
C_OBJS += $(MDLS)/common_impls/common/common.o
C_OBJS += $(MDLS)/common_impls/tools/utils.o
C_OBJS += $(MDLS)/common_impls/blm/blm.o
C_OBJS += $(MDLS)/common_impls/hash_functions/hashFunctions.o
C_OBJS += $(MDLS)/common_impls/ht/hashTable.o
C_OBJS += $(MDLS)/common_impls/list/linkedList.o
C_OBJS += $(MDLS)/common_impls/tools/converters.o
C_OBJS += $(MDLS)/common_impls/tools/logger.o
C_OBJS += $(MDLS)/common_impls/tools/errorChecking.o
C_OBJS += $(MDLS)/common_impls/tools/serialization.o
C_OBJS += $(MDLS)/common_impls/network/network.o
