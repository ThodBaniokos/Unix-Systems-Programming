TRV_M_OBJS += $(MDLS)/trvl_mon/helpers/travelMonitorHelpers.o
TRV_M_OBJS += $(MDLS)/trvl_mon/ioAPI/commands.o
TRV_M_OBJS += $(MDLS)/trvl_mon/ioAPI/ioAPI.o
TRV_M_OBJS += $(MDLS)/trvl_mon/signal/sig_actions.o
TRV_M_OBJS += $(MDLS)/trvl_mon/tools/serialization.o
TRV_M_OBJS += $(MDLS)/trvl_mon/tools/errorChecking.o

MON_OBJS += $(MDLS)/mon/helpers/monitorHelpers.o
MON_OBJS += $(MDLS)/mon/skip/skipList.o
MON_OBJS += $(MDLS)/mon/tools/answer.o
MON_OBJS += $(MDLS)/mon/tools/general.o
MON_OBJS += $(MDLS)/mon/ioAPI/mon_commands.o
MON_OBJS += $(MDLS)/mon/person/person.o
MON_OBJS += $(MDLS)/mon/signal/mon_sig_actions.o

C_OBJS += $(MDLS)/common_impls/ipc_protocol/ipc_protocol.o
C_OBJS += $(MDLS)/common_impls/common/common.o
C_OBJS += $(MDLS)/common_impls/tools/utils.o
C_OBJS += $(MDLS)/common_impls/blm/blm.o
C_OBJS += $(MDLS)/common_impls/hash_functions/hashFunctions.o
C_OBJS += $(MDLS)/common_impls/ht/hashTable.o
C_OBJS += $(MDLS)/common_impls/list/linkedList.o
C_OBJS += $(MDLS)/common_impls/tools/converters.o
C_OBJS += $(MDLS)/common_impls/tools/logger.o