!                   **************************
                    SUBROUTINE DEALL_TELEMAC2D
!                   **************************
     & (FULL_DEALL)
!
!
!***********************************************************************
! TELEMAC2D   V6P3                                   21/08/2010
!***********************************************************************
!
!brief    Memory allocation of structures, aliases, blocks...
!
!history  J-M HERVOUET (LNHE)
!+        02/04/2009
!+
!+   T2D_FILES(T2DGEO)%LU REPLACES NGEO
!
!~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
!~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
!
      USE BIEF
      USE DECLARATIONS_TELEMAC
      USE TPXO, ONLY : DEALLOC_TPXO
      USE TEL4DEL, ONLY : DEALLOC_TEL4DEL
      USE METEO_TELEMAC, ONLY : DEALLOC_METEO
      USE OILSPILL
      USE DECLARATIONS_TELEMAC2D
!
      USE DECLARATIONS_SPECIAL
      IMPLICIT NONE
!
!+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
!
      LOGICAL, INTENT(IN) :: FULL_DEALL
!
!+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
!
      INTEGER :: I
!
!-----------------------------------------------------------------------
!
!     THOMPS
      IF(.NOT.INIT_THOMPS) THEN
        CALL BIEF_DEALLOBJ(FNCAR1)
        CALL BIEF_DEALLOBJ(FTILD1)
        INIT_THOMPS = .TRUE.
      ENDIF
      CALL BIEF_DEALLOBJ(VARSOR)
      CALL BIEF_DEALLOBJ(PRIVE)
      CALL BIEF_DEALLOBJ(TB)
!     Deallocate the mesh structure
      IF(FULL_DEALL) THEN
        CALL DEALMESH(MESH)
      ENDIF
      NULLIFY(IKLE)
      NULLIFY(X)
      NULLIFY(Y)
      NULLIFY(NELEM)
      NULLIFY(NELMAX)
      NULLIFY(NPTFR)
      NULLIFY(NPTFRX)
      NULLIFY(TYPELM)
      NULLIFY(NPOIN)
      NULLIFY(NPMAX)
      NULLIFY(MXPTVS)
      NULLIFY(LV)
!
      DEALLOCATE(W)
!     DOUBLE PRECISION
      DEALLOCATE(MASTEN)
      DEALLOCATE(MASTOU)
      DEALLOCATE(MASTRAIN)
      DEALLOCATE(MASSOU)
      DEALLOCATE(MASTR0)
      DEALLOCATE(MASTR2)
      DEALLOCATE(FLUTSOR)
      DEALLOCATE(FLUTENT)
      IF(EQUA(1:15).EQ.'SAINT-VENANT VF') THEN
        DEALLOCATE(VNX1)
        DEALLOCATE(VNY1)
        DEALLOCATE(YESNOFR)
      ENDIF
!     LOGICAL
      IF(NTRAC.GT.0) THEN
        IF(ALLOCATED(YASMI)) DEALLOCATE(YASMI)
      END IF
!
!     Deallocating all the blocks first
!
      CALL BIEF_DEALLOBJ(FN)
      CALL BIEF_DEALLOBJ(F )
      CALL BIEF_DEALLOBJ(HT       )
      CALL BIEF_DEALLOBJ(FLUXT    )
      CALL BIEF_DEALLOBJ(FLUXT_OLD)
      CALL BIEF_DEALLOBJ(FLUHTEMP )
      CALL BIEF_DEALLOBJ(T     )
      CALL BIEF_DEALLOBJ(UNK)
      CALL BIEF_DEALLOBJ(DIRBOR)
      CALL BIEF_DEALLOBJ(MAT)
      CALL BIEF_DEALLOBJ(RHS)
      CALL BIEF_DEALLOBJ(TB2)
      CALL BIEF_DEALLOBJ(MSKSEC)
      CALL BIEF_DEALLOBJ(MASK)
      CALL BIEF_DEALLOBJ(FTILD)
      CALL BIEF_DEALLOBJ(FTILD2)
      CALL BIEF_DEALLOBJ(FNCAR)
      CALL BIEF_DEALLOBJ(FNCAR2)
      IF(FULL_DEALL) THEN
        CALL BIEF_DEALLOBJ(TEXP  )
        CALL BIEF_DEALLOBJ(TTILD )
        CALL BIEF_DEALLOBJ(TN    )
      ENDIF
      CALL BIEF_DEALLOBJ(TIMP  )
      CALL BIEF_DEALLOBJ(TSCEXP)
      CALL BIEF_DEALLOBJ(VISCT )
      CALL BIEF_DEALLOBJ(MASKTR)
      IF(FULL_DEALL) THEN
        CALL BIEF_DEALLOBJ(LITBOR)
        CALL BIEF_DEALLOBJ(TBOR  )
        CALL BIEF_DEALLOBJ(ATBOR )
        CALL BIEF_DEALLOBJ(BTBOR )
      ENDIF
      CALL BIEF_DEALLOBJ(TBUS )
      CALL BIEF_DEALLOBJ(VARCL)
      CALL BIEF_DEALLOBJ(SMTR     )
      CALL BIEF_DEALLOBJ(AMPL)
      CALL BIEF_DEALLOBJ(PHAS)
!
      CALL BIEF_DEALLOBJ(S)
!
      CALL BIEF_DEALLOBJ(U)
      CALL BIEF_DEALLOBJ(V)
!
      CALL BIEF_DEALLOBJ(UTILD)
      CALL BIEF_DEALLOBJ(VTILD)
      CALL BIEF_DEALLOBJ(HTILD)
!
      IF(FULL_DEALL) THEN
        CALL BIEF_DEALLOBJ(H)
      ENDIF
      CALL BIEF_DEALLOBJ(U)
      CALL BIEF_DEALLOBJ(V)
      CALL BIEF_DEALLOBJ(UN)
      CALL BIEF_DEALLOBJ(VN)
      CALL BIEF_DEALLOBJ(HN)
!
      CALL BIEF_DEALLOBJ(DH)
      CALL BIEF_DEALLOBJ(DU)
      CALL BIEF_DEALLOBJ(DV)
      CALL BIEF_DEALLOBJ(DHN)
!
!
      IF(FULL_DEALL) THEN
        CALL BIEF_DEALLOBJ(HBOR    )
        CALL BIEF_DEALLOBJ(UBOR    )
        CALL BIEF_DEALLOBJ(VBOR    )
      ENDIF
      CALL BIEF_DEALLOBJ(AUBOR   )
      CALL BIEF_DEALLOBJ(CFBOR   )
      CALL BIEF_DEALLOBJ(UETUTA  )
      CALL BIEF_DEALLOBJ(FLBOR   )
      CALL BIEF_DEALLOBJ(FLBORTRA)
!
      CALL BIEF_DEALLOBJ(HBTIDE )
      CALL BIEF_DEALLOBJ(UBTIDE )
      CALL BIEF_DEALLOBJ(VBTIDE )
      CALL BIEF_DEALLOBJ(NUMTIDE)
!
!
      IF(FULL_DEALL) THEN
        CALL BIEF_DEALLOBJ(ZF    )
      ENDIF
      CALL BIEF_DEALLOBJ(ZFE   )
      CALL BIEF_DEALLOBJ(VISC  )
      IF(OPDVIT.EQ.2.OR.(NTRAC.GT.0.AND.OPDTRA.EQ.2)) THEN
        CALL BIEF_DEALLOBJ(VISC_S)
      ENDIF
      CALL BIEF_DEALLOBJ(CHESTR)
      CALL BIEF_DEALLOBJ(RO    )
      CALL BIEF_DEALLOBJ(FU    )
      CALL BIEF_DEALLOBJ(FV    )
      CALL BIEF_DEALLOBJ(FXWAVE)
      CALL BIEF_DEALLOBJ(FYWAVE)
!
      CALL DEALLOC_METEO()
!
      CALL BIEF_DEALLOBJ(BM1S)
      CALL BIEF_DEALLOBJ(BM2S)
      CALL BIEF_DEALLOBJ(CV1S)
      CALL BIEF_DEALLOBJ(TM1 )
      CALL BIEF_DEALLOBJ(MBOR)
      CALL BIEF_DEALLOBJ(AM1)
      CALL BIEF_DEALLOBJ(BM1)
      CALL BIEF_DEALLOBJ(BM2)
      CALL BIEF_DEALLOBJ(CM1)
      CALL BIEF_DEALLOBJ(AM2)
      CALL BIEF_DEALLOBJ(A23)
      CALL BIEF_DEALLOBJ(CM2)
      CALL BIEF_DEALLOBJ(A32)
      CALL BIEF_DEALLOBJ(AM3)
!
      CALL BIEF_DEALLOBJ(W1 )
      CALL BIEF_DEALLOBJ(W2 )
      CALL BIEF_DEALLOBJ(W1DEB)
      CALL BIEF_DEALLOBJ(CV1)
      CALL BIEF_DEALLOBJ(CV2)
      CALL BIEF_DEALLOBJ(CV3)
!
      CALL BIEF_DEALLOBJ(SMH   )
      CALL BIEF_DEALLOBJ(UCONV )
      CALL BIEF_DEALLOBJ(VCONV )
      CALL BIEF_DEALLOBJ(HPROP )
      CALL BIEF_DEALLOBJ(VOLU2D)
      CALL BIEF_DEALLOBJ(V2DPAR)
      CALL BIEF_DEALLOBJ(UNSV2D)
      CALL BIEF_DEALLOBJ(XLAG  )
      CALL BIEF_DEALLOBJ(YLAG  )
      CALL BIEF_DEALLOBJ(SHPLAG)
!
!
      CALL BIEF_DEALLOBJ(TE1)
      CALL BIEF_DEALLOBJ(TE2)
      CALL BIEF_DEALLOBJ(TE3)
      CALL BIEF_DEALLOBJ(ZFLATS)
      CALL BIEF_DEALLOBJ(TE4)
      CALL BIEF_DEALLOBJ(TE5)
!
!
      CALL BIEF_DEALLOBJ(MASKEL)
      CALL BIEF_DEALLOBJ(MASKPT)
!
!
      CALL BIEF_DEALLOBJ(CF    )
      CALL BIEF_DEALLOBJ(NKFROT)
      IF(FULL_DEALL) THEN
        CALL BIEF_DEALLOBJ(CHBORD)
      ENDIF
      IF(FRICTB) THEN
        DO I=1,NZONMX
          DEALLOCATE(FRTAB%ADR(I)%P)
        ENDDO
        DEALLOCATE(FRTAB%ADR)
      ENDIF
!
      CALL BIEF_DEALLOBJ(KFROPT)
      CALL BIEF_DEALLOBJ(NDEFMA)
      CALL BIEF_DEALLOBJ(VCOEFF)
      CALL BIEF_DEALLOBJ(VEGLAW)
      CALL BIEF_DEALLOBJ(NDEF_B)
      CALL BIEF_DEALLOBJ(KFRO_B)
      CALL BIEF_DEALLOBJ(AK    )
      CALL BIEF_DEALLOBJ(EP    )
      CALL BIEF_DEALLOBJ(AKN   )
      CALL BIEF_DEALLOBJ(EPN   )
      CALL BIEF_DEALLOBJ(AKTILD)
      CALL BIEF_DEALLOBJ(EPTILD)
      CALL BIEF_DEALLOBJ(KBOR  )
      CALL BIEF_DEALLOBJ(EBOR  )
      CALL BIEF_DEALLOBJ(UDEL  )
      CALL BIEF_DEALLOBJ(VDEL  )
      CALL BIEF_DEALLOBJ(DM1   )
      CALL BIEF_DEALLOBJ(ZCONV )
      CALL BIEF_DEALLOBJ(FLODEL)
      CALL BIEF_DEALLOBJ(FLULIM)
      CALL BIEF_DEALLOBJ(PLUIE )
      CALL BIEF_DEALLOBJ(ACCROF)
      CALL BIEF_DEALLOBJ(ACCR  )
      CALL BIEF_DEALLOBJ(POTMAXRET)
      CALL BIEF_DEALLOBJ(IABST )
      CALL BIEF_DEALLOBJ(CN    )
      CALL BIEF_DEALLOBJ(ZFSLOP)
      CALL BIEF_DEALLOBJ(DIRMOY)
      CALL BIEF_DEALLOBJ(HM0   )
      CALL BIEF_DEALLOBJ(TPR5  )
      CALL BIEF_DEALLOBJ(ORBVEL)
!
!
!
      CALL BIEF_DEALLOBJ(XFLOT )
      CALL BIEF_DEALLOBJ(YFLOT )
      CALL BIEF_DEALLOBJ(SHPFLO)
!
!
      CALL BIEF_DEALLOBJ(ENTBUS)
      CALL BIEF_DEALLOBJ(SORBUS)
      CALL BIEF_DEALLOBJ(ALTBUS)
      CALL BIEF_DEALLOBJ(CSBUS )
      CALL BIEF_DEALLOBJ(CEBUS )
      CALL BIEF_DEALLOBJ(ANGBUS)
      CALL BIEF_DEALLOBJ(LBUS  )
      CALL BIEF_DEALLOBJ(UBUS  )
      CALL BIEF_DEALLOBJ(VBUS  )
      CALL BIEF_DEALLOBJ(DBUS  )
      CALL BIEF_DEALLOBJ(LRGBUS)
      CALL BIEF_DEALLOBJ(HAUBUS)
      CALL BIEF_DEALLOBJ(SECBUS)
      CALL BIEF_DEALLOBJ(CLPBUS)
      CALL BIEF_DEALLOBJ(CIRC  )
      CALL BIEF_DEALLOBJ(CV    )
      CALL BIEF_DEALLOBJ(C56   )
      CALL BIEF_DEALLOBJ(CV5   )
      CALL BIEF_DEALLOBJ(C5    )
      CALL BIEF_DEALLOBJ(CTRASH)
      CALL BIEF_DEALLOBJ(FRICBUS)
      CALL BIEF_DEALLOBJ(LONGBUS)
      CALL BIEF_DEALLOBJ(SEC_TAU)
      CALL BIEF_DEALLOBJ(SEC_R)
!
!
!
      CALL BIEF_DEALLOBJ(IFAMAS)
      IF(FULL_DEALL) THEN
        CALL BIEF_DEALLOBJ(LIUBOR)
        CALL BIEF_DEALLOBJ(LIVBOR)
        CALL BIEF_DEALLOBJ(LIHBOR)
      ENDIF
      CALL BIEF_DEALLOBJ(CLU   )
      CALL BIEF_DEALLOBJ(CLV   )
      CALL BIEF_DEALLOBJ(CLH   )
      CALL BIEF_DEALLOBJ(BOUNDARY_COLOUR)
      CALL BIEF_DEALLOBJ(NUMLIQ)
      CALL BIEF_DEALLOBJ(LIMKEP)
      CALL BIEF_DEALLOBJ(LIMPRO)
      CALL BIEF_DEALLOBJ(LIMTRA)
      CALL BIEF_DEALLOBJ(SECMOU)
      CALL BIEF_DEALLOBJ(IT1   )
      CALL BIEF_DEALLOBJ(IT2   )
      CALL BIEF_DEALLOBJ(IT3   )
      CALL BIEF_DEALLOBJ(IT4   )
      CALL BIEF_DEALLOBJ(DEBFLO)
      CALL BIEF_DEALLOBJ(FINFLO)
      CALL BIEF_DEALLOBJ(ELTFLO)
      CALL BIEF_DEALLOBJ(TAGFLO)
      CALL BIEF_DEALLOBJ(CLSFLO)
      CALL BIEF_DEALLOBJ(DEBLAG)
      CALL BIEF_DEALLOBJ(FINLAG)
      CALL BIEF_DEALLOBJ(ELTLAG)
      CALL BIEF_DEALLOBJ(ZONE  )
!
!
      CALL BIEF_DEALLOBJ(QU       )
      CALL BIEF_DEALLOBJ(QV       )
      CALL BIEF_DEALLOBJ(HSTOK    )
      CALL BIEF_DEALLOBJ(HCSTOK   )
      CALL BIEF_DEALLOBJ(LOGFR    )
      CALL BIEF_DEALLOBJ(HC       )
      CALL BIEF_DEALLOBJ(DSZ      )
      CALL BIEF_DEALLOBJ(FLUX_OLD )
      IF(EQUA(1:15).EQ.'SAINT-VENANT VF') THEN
        CALL BIEF_DEALLOBJ(NEISEG )
        CALL BIEF_DEALLOBJ(CORR_I )
        CALL BIEF_DEALLOBJ(CORR_J )
        CALL BIEF_DEALLOBJ(CORR_ZL)
        CALL BIEF_DEALLOBJ(CORR_ZR)
        CALL BIEF_DEALLOBJ(CORR_HL)
        CALL BIEF_DEALLOBJ(CORR_HR)
        CALL BIEF_DEALLOBJ(CORR_UL)
        CALL BIEF_DEALLOBJ(CORR_UR)
        CALL BIEF_DEALLOBJ(CORR_VL)
        CALL BIEF_DEALLOBJ(CORR_VR)
        CALL BIEF_DEALLOBJ(ALRTPF )
        CALL BIEF_DEALLOBJ(FLUX   )
      ENDIF
      CALL BIEF_DEALLOBJ(H0   )
      CALL BIEF_DEALLOBJ(MAXZ )
      CALL BIEF_DEALLOBJ(TMAXZ)
      CALL BIEF_DEALLOBJ(MAXV )
      CALL BIEF_DEALLOBJ(TMAXV)
!
!     Deallocate some global variables allocated
!     at some point in the code
!
      IF(ALLOCATED(W)) THEN
        DEALLOCATE(W)
      ENDIF
      IF(ALLOCATED(QZ)) THEN
        DEALLOCATE(QZ)
      ENDIF
      IF(ALLOCATED(CHAIN)) THEN
        DEALLOCATE(CHAIN)
      ENDIF
!     HANDLING OLD SAVED VARIABLES
!
!     MAJTRAC
      IF(DEJA_MT) THEN
        DEALLOCATE(DST_MT)
        DEALLOCATE(DSP_MT)
        DEALLOCATE(DSM_MT)
        DEALLOCATE(CORRT_MT)
        DEALLOCATE(GRADI_MT)
        DEALLOCATE(GRADJ_MT)
        DEALLOCATE(GRADJI_MT)
        DEALLOCATE(GRADIJ_MT)
        DEJA_MT = .FALSE.
      ENDIF
!     READ_FIC_SOURCE
      IF(DEJA_RFS) THEN
        DEALLOCATE(INFIC_RFS)
        DEALLOCATE(TIME_RFS)
        DEJA_RFS = .FALSE.
      ENDIF
!     FLUXPR_TELEMAC2D
      IF(.NOT.INIT_FPR.AND.NCSIZE.GT.1) THEN
        DEALLOCATE(WORK_FPR)
        INIT_FPR=.TRUE.
      ENDIF
!     TESTEUR
      IF(DEJA_TESTEUR) THEN
        DEALLOCATE(FLUX_TESTEUR)
        DEJA_TESTEUR = .FALSE.
      ENDIF
!     SECOND ORDER
      IF(DEJA_FC) THEN
        DEALLOCATE(DSH_FC)
        DEALLOCATE(DSU_FC)
        DEALLOCATE(DSV_FC)
        DEALLOCATE(DSP_FC)
        DEALLOCATE(DSM_FC)
        DEALLOCATE(DSZ_FC)
        DEALLOCATE(CORR_FC)
        DEALLOCATE(DTLL_FC)
        DEALLOCATE(GRADI_FC)
        DEALLOCATE(GRADJ_FC)
        DEALLOCATE(GRADJI_FC)
        DEALLOCATE(GRADIJ_FC)
        DEJA_FC = .FALSE.
      ENDIF
!     READ_FR_FRLIQ
      IF(DEJA_RFF) THEN
        DEALLOCATE(INFIC_RFF)
        DEALLOCATE(TIME_RFF)
        DEJA_RFF = .FALSE.
      ENDIF
!     SPECTRE
      IF(DEJA) THEN
        DEALLOCATE(AM)
        DEALLOCATE(BM)
        DEALLOCATE(HA)
        DEJA = .FALSE.
      ENDIF
!     BORD_TIDAL_BC
      IF(DEJA_TBC) THEN
        DEALLOCATE(FIRSTTIDE)
        DEALLOCATE(LASTTIDE)
        DEJA_TBC = .FALSE.
      ENDIF
!
      IF(DEJA_FS) THEN
        DEALLOCATE(VLX_FS)
        DEALLOCATE(VOLNEG_FS)
        DEALLOCATE(VOLPOS_FS)
        DEALLOCATE(NSEG_FS)
        DEALLOCATE(LISTE_FS)
        DEJA_FS = .FALSE.
      ENDIF
!     BORD_TIDE
      IF(DEJA_BT) THEN
        IF(ALLOCATED(FIRSTTIDE_BT)) DEALLOCATE(FIRSTTIDE_BT)
        IF(ALLOCATED(LASTTIDE_BT)) DEALLOCATE(LASTTIDE_BT)
        IF(ALLOCATED(SHIFTTIDE_BT)) DEALLOCATE(SHIFTTIDE_BT)
        IF(ALLOCATED(AHM2)) DEALLOCATE(AHM2)
        IF(ALLOCATED(PHM2)) DEALLOCATE(PHM2)
        IF(ALLOCATED(AHS2)) DEALLOCATE(AHS2)
        IF(ALLOCATED(PHS2)) DEALLOCATE(PHS2)
        IF(ALLOCATED(AHN2)) DEALLOCATE(AHN2)
        IF(ALLOCATED(PHN2)) DEALLOCATE(PHN2)
        IF(ALLOCATED(AHM4)) DEALLOCATE(AHM4)
        IF(ALLOCATED(PHM4)) DEALLOCATE(PHM4)
        IF(ALLOCATED(AUM2)) DEALLOCATE(AUM2)
        IF(ALLOCATED(PUM2)) DEALLOCATE(PUM2)
        IF(ALLOCATED(AUS2)) DEALLOCATE(AUS2)
        IF(ALLOCATED(PUS2)) DEALLOCATE(PUS2)
        IF(ALLOCATED(AUN2)) DEALLOCATE(AUN2)
        IF(ALLOCATED(PUN2)) DEALLOCATE(PUN2)
        IF(ALLOCATED(AUM4)) DEALLOCATE(AUM4)
        IF(ALLOCATED(PUM4)) DEALLOCATE(PUM4)
        IF(ALLOCATED(AVM2)) DEALLOCATE(AVM2)
        IF(ALLOCATED(PVM2)) DEALLOCATE(PVM2)
        IF(ALLOCATED(AVS2)) DEALLOCATE(AVS2)
        IF(ALLOCATED(PVS2)) DEALLOCATE(PVS2)
        IF(ALLOCATED(AVN2)) DEALLOCATE(AVN2)
        IF(ALLOCATED(PVN2)) DEALLOCATE(PVN2)
        IF(ALLOCATED(AVM4)) DEALLOCATE(AVM4)
        IF(ALLOCATED(PVM4)) DEALLOCATE(PVM4)
        IF(ALLOCATED(FHXM2)) DEALLOCATE(FHXM2)
        IF(ALLOCATED(FHYM2)) DEALLOCATE(FHYM2)
        IF(ALLOCATED(FHXS2)) DEALLOCATE(FHXS2)
        IF(ALLOCATED(FHYS2)) DEALLOCATE(FHYS2)
        IF(ALLOCATED(FHXN2)) DEALLOCATE(FHXN2)
        IF(ALLOCATED(FHYN2)) DEALLOCATE(FHYN2)
        IF(ALLOCATED(FHXM4)) DEALLOCATE(FHXM4)
        IF(ALLOCATED(FHYM4)) DEALLOCATE(FHYM4)
        IF(ALLOCATED(FUXM2)) DEALLOCATE(FUXM2)
        IF(ALLOCATED(FUYM2)) DEALLOCATE(FUYM2)
        IF(ALLOCATED(FUXS2)) DEALLOCATE(FUXS2)
        IF(ALLOCATED(FUYS2)) DEALLOCATE(FUYS2)
        IF(ALLOCATED(FUXN2)) DEALLOCATE(FUXN2)
        IF(ALLOCATED(FUYN2)) DEALLOCATE(FUYN2)
        IF(ALLOCATED(FUXM4)) DEALLOCATE(FUXM4)
        IF(ALLOCATED(FUYM4)) DEALLOCATE(FUYM4)
        IF(ALLOCATED(FVXM2)) DEALLOCATE(FVXM2)
        IF(ALLOCATED(FVYM2)) DEALLOCATE(FVYM2)
        IF(ALLOCATED(FVXS2)) DEALLOCATE(FVXS2)
        IF(ALLOCATED(FVYS2)) DEALLOCATE(FVYS2)
        IF(ALLOCATED(FVXN2)) DEALLOCATE(FVXN2)
        IF(ALLOCATED(FVYN2)) DEALLOCATE(FVYN2)
        IF(ALLOCATED(FVXM4)) DEALLOCATE(FVXM4)
        IF(ALLOCATED(FVYM4)) DEALLOCATE(FVYM4)
        DEJA_BT = .FALSE.
      ENDIF
!     BORD_TIDE_MISC
      IF(DEJA_BTM) THEN
        DEALLOCATE(FIRSTTIDE_BTM)
        DEALLOCATE(LASTTIDE_BTM)
        DEALLOCATE(SHIFTTIDE_BTM)
        DEALLOCATE(INDW_BTM)
        IF(ALLOCATED(INDW2_BTM)) DEALLOCATE(INDW2_BTM)
        IF(ALLOCATED(INDW3_BTM)) DEALLOCATE(INDW3_BTM)
        IF(ALLOCATED(NAMEWAVE_BTM)) DEALLOCATE(NAMEWAVE_BTM)
        IF(ALLOCATED(AH_BTM)) DEALLOCATE(AH_BTM)
        IF(ALLOCATED(PH_BTM)) DEALLOCATE(PH_BTM)
        IF(ALLOCATED(AU_BTM)) DEALLOCATE(AU_BTM)
        IF(ALLOCATED(PU_BTM)) DEALLOCATE(PU_BTM)
        IF(ALLOCATED(AV_BTM)) DEALLOCATE(AV_BTM)
        IF(ALLOCATED(PV_BTM)) DEALLOCATE(PV_BTM)
        IF(ALLOCATED(LON_BTM)) DEALLOCATE(LON_BTM)
        IF(ALLOCATED(LAT_BTM)) DEALLOCATE(LAT_BTM)
        IF(ALLOCATED(UPV_BTM)) DEALLOCATE(UPV_BTM)
        IF(ALLOCATED(FF_BTM)) DEALLOCATE(FF_BTM)
        IF(ALLOCATED(OMEGA_BTM)) DEALLOCATE(OMEGA_BTM)
        IF(ALLOCATED(PHCALHW_BTM)) DEALLOCATE(PHCALHW_BTM)
        DEJA_BTM = .FALSE.
      ENDIF
!
!     OILSPILL
!
      IF(INIT_3D) THEN
        CALL BIEF_DEALLOBJ(UCONV_OIL)
        CALL BIEF_DEALLOBJ(VCONV_OIL)
        DO I=1,NFLOT_MAX
          DEALLOCATE(PARTICULES(I)%COMPO)
          DEALLOCATE(PARTICULES(I)%HAP)
        ENDDO
        DEALLOCATE(PARTICULES)
        DEALLOCATE(TB_COMPO_3D)
        DEALLOCATE(FM_COMPO_3D)
        DEALLOCATE(TB_HAP_3D)
        DEALLOCATE(FM_HAP_3D)
        DEALLOCATE(SOLU_3D)
        DEALLOCATE(KDISS_3D)
        DEALLOCATE(KVOL_3D)
        INIT_3D = .FALSE.
      ENDIF
      IF(INIT_2D) THEN
        CALL BIEF_DEALLOBJ(UCONV_OIL)
        CALL BIEF_DEALLOBJ(VCONV_OIL)
        DO I=1,NFLOT_MAX
          DEALLOCATE(PARTICULES(I)%COMPO)
          DEALLOCATE(PARTICULES(I)%HAP)
        ENDDO
        DEALLOCATE(PARTICULES)
        DEALLOCATE(TB_COMPO_2D)
        DEALLOCATE(FM_COMPO_2D)
        DEALLOCATE(TB_HAP_2D)
        DEALLOCATE(FM_HAP_2D)
        DEALLOCATE(SOLU_2D)
        DEALLOCATE(KDISS_2D)
        DEALLOCATE(KVOL_2D)
        INIT_2D = .FALSE.
      ENDIF
      DEJA_DERIVE2 = .FALSE.
!
!     TEL4DEL
!
      CALL DEALLOC_TEL4DEL()
!
!     TPXO
!
      CALL DEALLOC_TPXO()
!
!     Breach
!
      IF(DEJALU_BREACH) THEN
        CALL BIEF_DEALLOBJ(INDBR)
        CALL BIEF_DEALLOBJ(DKAXCR)
        CALL BIEF_DEALLOBJ(DKAYCR)
        CALL BIEF_DEALLOBJ(PONDSB)
        !
        CALL BIEF_DEALLOBJ(OPTNBR)
        CALL BIEF_DEALLOBJ(OPTERO)
        CALL BIEF_DEALLOBJ(TDECBR)
        CALL BIEF_DEALLOBJ(DURBR )
        CALL BIEF_DEALLOBJ(ZFINBR)
        CALL BIEF_DEALLOBJ(ZDECBR)
        CALL BIEF_DEALLOBJ(ZCRBR )
        CALL BIEF_DEALLOBJ(POLWDT)
        CALL BIEF_DEALLOBJ(NUMPSD)
        CALL BIEF_DEALLOBJ(NBNDBR)
        CALL BIEF_DEALLOBJ(NPONBR)
        CALL BIEF_DEALLOBJ(CURBRW)
        CALL BIEF_DEALLOBJ(FINBRW)
        CALL BIEF_DEALLOBJ(INIBRW)
        CALL BIEF_DEALLOBJ(DEPTHN)
        DEJALU_BREACH = .FALSE.
      ENDIF
      !

!
!     Weirs (lecsng)
!
      IF(NWEIRS.GT.0) THEN
        IF(TYPSEUIL.EQ.1) THEN
          CALL BIEF_DEALLOBJ(NPSING)
          CALL BIEF_DEALLOBJ(NDGA1)
          CALL BIEF_DEALLOBJ(NDGB1)
          CALL BIEF_DEALLOBJ(ZDIG)
          CALL BIEF_DEALLOBJ(PHIDIG)
        ELSE
          DEALLOCATE(WEIRS )
          IF(ALLOCATED(WEIRS_PROC)) DEALLOCATE(WEIRS_PROC)
          IF(ALLOCATED(WNODES)) DEALLOCATE(WNODES)
          IF(ALLOCATED(WNODES_PROC)) DEALLOCATE(WNODES_PROC)
          IF(ALLOCATED(WN_SEND)) DEALLOCATE(WN_SEND)
          IF(ALLOCATED(WN_SEND_PROC)) DEALLOCATE(WN_SEND_PROC)
          IF(ALLOCATED(W_BUF_RECV)) DEALLOCATE(W_BUF_RECV)
          IF(ALLOCATED(W_BUF_SEND)) DEALLOCATE(W_BUF_SEND)
          IF(ALLOCATED(WT_BUF_RECV)) DEALLOCATE(WT_BUF_RECV)
          IF(ALLOCATED(WT_BUF_SEND)) DEALLOCATE(WT_BUF_SEND)
          IF(ALLOCATED(WH_BUF_RECV)) DEALLOCATE(WH_BUF_RECV)
          IF(ALLOCATED(WH_BUF_SEND)) DEALLOCATE(WH_BUF_SEND)
          IF(ALLOCATED(WEIR_BREACH_START)) DEALLOCATE(WEIR_BREACH_START)
          IF(ALLOCATED(WEIR_BREACH_END)) DEALLOCATE(WEIR_BREACH_END)
        ENDIF
      ENDIF
!
!     flusec_t2d.f
!
      IF(DEJA_FLUSECT2D) THEN
        DO I = 1,NUMBEROFLINES_FLUSECT2D
          DEALLOCATE(FLUXLINEDATA_FLUSECT2D(I)%SECTIONIDS)
          DEALLOCATE(FLUXLINEDATA_FLUSECT2D(I)%DIRECTION)
        ENDDO
        DEALLOCATE(FLUXLINEDATA_FLUSECT2D)
        DEALLOCATE(FLX_FLUSECT2D)
        DEALLOCATE(VOLFLUX_FLUSECT2D)
        DEJA_FLUSECT2D = .FALSE.
      ENDIF

!
!     prosou.f
!
      PROSOU_DEJALU = .FALSE.
!
!     DEALLOCATION OF SOURCES REGIONS
!
      IF(ALLOCATED(XCOO)) DEALLOCATE(XCOO)
      IF(ALLOCATED(YCOO)) DEALLOCATE(YCOO)
      IF(ALLOCATED(PTS_REG)) DEALLOCATE(PTS_REG)
      IF(ALLOCATED(AREA_P)) DEALLOCATE(AREA_P)
      IF(ALLOCATED(TNP)) DEALLOCATE(TNP)
      IF(ALLOCATED(PT_IN_POLY)) DEALLOCATE(PT_IN_POLY)
!
!     DEALLOCATION OF SOME VECTORS FOR BREACH
!
      IF(BRECHE) THEN
        DEALLOCATE(DIKE1)
        DEALLOCATE(DIKE2)
        DEALLOCATE(DIKE3)
        DEALLOCATE(FT1)
        DEALLOCATE(VELS1)
        DEALLOCATE(VELS2)
        DEALLOCATE(UC)
        DEALLOCATE(F1)
        DEALLOCATE(F2)
        DEALLOCATE(DF)
        DEALLOCATE(NBLS)
      ENDIF
!
!     Deallocating arrays allocated in lecdon_telemac2d
!
      IF(FULL_DEALL) THEN
        DEALLOCATE(FRTYPE)
        DEALLOCATE(PROVEL)
        DEALLOCATE(PTS_CURVES)
        DEALLOCATE(STA_DIS_CURVES)
        DEALLOCATE(BND_TIDE)
        DEALLOCATE(DEBIT)
        DEALLOCATE(COTE)
        DEALLOCATE(VITES)
        DEALLOCATE(TRACER)
        DEALLOCATE(FLUX_BOUNDARIES)
        DEALLOCATE(ISCE)
        DEALLOCATE(XSCE)
        DEALLOCATE(YSCE)
        DEALLOCATE(DSCE)
        DEALLOCATE(DSCE2)
        DEALLOCATE(TSCE)
        DEALLOCATE(TSCE2)
        DEALLOCATE(USCE)
        DEALLOCATE(VSCE)
        DEALLOCATE(OKDEBSCE)
        DEALLOCATE(OKTRSCE)
        DEALLOCATE(OKVIT)
        DEALLOCATE(OKQ)
        DEALLOCATE(OKSL)
        DEALLOCATE(OKTR)
        DEALLOCATE(ICONVFT)
        DEALLOCATE(TRAIN)
        DEALLOCATE(OPTADV_TR)
        DEALLOCATE(SLVTRA)
        DEALLOCATE(TRAC0)
        DEALLOCATE(NAMETRAC)
        DEALLOCATE(DIFNU)
        DEALLOCATE(MVIST)
        DEALLOCATE(CTRLSC)
        IF( ALGAE ) THEN
          DEALLOCATE(YALGAE)
          DEALLOCATE(DALGAE)
          DEALLOCATE(RALGAE)
          DEALLOCATE(EALGAE)
          DEALLOCATE(TALGAE)
        ENDIF
      ENDIF

      IF(INCLUS(COUPLING,'TOMAWAC2')) THEN
        CALL BIEF_DEALLOBJ(TEL2TOM)
        CALL BIEF_DEALLOBJ(TOM2TEL)
      ENDIF

      ! Resetting initial values of variables
      OLD_METHOD_FPR=.FALSE.
      OLD_METHOD_FS=.FALSE.
      MSG_TAG = 5000
      MSG_TAG_TRA = 5000
!
!=======================================================================
!
! WRITES OUT TO LISTING :
!
      IF(LISTIN) THEN
        WRITE(LU,23)
      ENDIF
23    FORMAT(1X,///,21X,'*************************************',/,
     &21X,              '*    END OF MEMORY ORGANIZATION:    *',/,
     &21X,              '*************************************',/)
!
!-----------------------------------------------------------------------
!
      RETURN
      END SUBROUTINE DEALL_TELEMAC2D
