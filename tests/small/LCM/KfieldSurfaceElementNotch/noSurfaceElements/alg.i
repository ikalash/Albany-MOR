save all

disp_x = solution_X
disp_y = solution_Y
disp_z = solution_Z

fint_x = residual_X
fint_y = residual_Y 
fint_z = residual_Z

s11 = (1/8)*(CAUCHY_STRESS_01+CAUCHY_STRESS_10+CAUCHY_STRESS_19+CAUCHY_STRESS_28+CAUCHY_STRESS_37+CAUCHY_STRESS_46 >
              +CAUCHY_STRESS_55+CAUCHY_STRESS_64)
s12 = (1/8)*(CAUCHY_STRESS_02+CAUCHY_STRESS_11+CAUCHY_STRESS_20+CAUCHY_STRESS_29+CAUCHY_STRESS_38+CAUCHY_STRESS_47 >
              +CAUCHY_STRESS_56+CAUCHY_STRESS_65)
s13 = (1/8)*(CAUCHY_STRESS_03+CAUCHY_STRESS_12+CAUCHY_STRESS_21+CAUCHY_STRESS_30+CAUCHY_STRESS_39+CAUCHY_STRESS_48 >
              +CAUCHY_STRESS_57+CAUCHY_STRESS_66)
s22 = (1/8)*(CAUCHY_STRESS_05+CAUCHY_STRESS_14+CAUCHY_STRESS_23+CAUCHY_STRESS_32+CAUCHY_STRESS_41+CAUCHY_STRESS_50 >
              +CAUCHY_STRESS_59+CAUCHY_STRESS_68)
s23 = (1/8)*(CAUCHY_STRESS_06+CAUCHY_STRESS_15+CAUCHY_STRESS_24+CAUCHY_STRESS_33+CAUCHY_STRESS_42+CAUCHY_STRESS_51 >
              +CAUCHY_STRESS_60+CAUCHY_STRESS_69)
s33 = (1/8)*(CAUCHY_STRESS_09+CAUCHY_STRESS_18+CAUCHY_STRESS_27+CAUCHY_STRESS_36+CAUCHY_STRESS_45+CAUCHY_STRESS_54 >
              +CAUCHY_STRESS_63+CAUCHY_STRESS_72)

vm = (1./sqrt(2))*TMAG(s11,s22,s33,s12,s23,s13)
vm_1 = (1./sqrt(2))*TMAG(cauchy_stress_01,cauchy_stress_05,cauchy_stress_09,cauchy_stress_02,cauchy_stress_06,cauchy_stress_03)

hydro = (1./3.)*(s11 + s22 + s33)

fp11 = Fp_01
fp12 = Fp_02
fp13 = Fp_03
fp21 = Fp_04
fp22 = Fp_05
fp23 = Fp_06
fp31 = Fp_07
fp32 = Fp_08
fp33 = Fp_09

Jp_1 = fp11*(fp22*fp33 - fp32*fp23) - fp12*(fp21*fp33 - fp31*fp23) + fp13*(fp21*fp32 - fp31*fp22)

Yield_Surface_J_1 = Yield_Surface_1/J_1

surf_11_1 = surf_cauchy_stress_01
surf_12_1 = surf_cauchy_stress_02
surf_13_1 = surf_cauchy_stress_03
surf_22_1 = surf_cauchy_stress_05
surf_23_1 = surf_cauchy_stress_06
surf_33_1 = surf_cauchy_stress_09
surf_11_2 = surf_cauchy_stress_10
surf_12_2 = surf_cauchy_stress_11
surf_13_2 = surf_cauchy_stress_12
surf_22_2 = surf_cauchy_stress_14
surf_23_2 = surf_cauchy_stress_15
surf_33_2 = surf_cauchy_stress_18
surf_11_3 = surf_cauchy_stress_19
surf_12_3 = surf_cauchy_stress_20
surf_13_3 = surf_cauchy_stress_21
surf_22_3 = surf_cauchy_stress_23
surf_23_3 = surf_cauchy_stress_24
surf_33_3 = surf_cauchy_stress_27
surf_11_4 = surf_cauchy_stress_28
surf_12_4 = surf_cauchy_stress_29
surf_13_4 = surf_cauchy_stress_30
surf_22_4 = surf_cauchy_stress_32
surf_23_4 = surf_cauchy_stress_33
surf_33_4 = surf_cauchy_stress_36

surf_hydro_1 = (1./3.)*(surf_11_1 + surf_22_1 + surf_33_1)
surf_hydro_2 = (1./3.)*(surf_11_2 + surf_22_2 + surf_33_2)
surf_hydro_3 = (1./3.)*(surf_11_3 + surf_22_3 + surf_33_3)
surf_hydro_4 = (1./3.)*(surf_11_4 + surf_22_4 + surf_33_4)

surf_s11 = (1./4.)*(surf_11_1 + surf_11_2 + surf_11_3 + surf_11_4)
surf_s12 = (1./4.)*(surf_12_1 + surf_12_2 + surf_12_3 + surf_12_4)
surf_s13 = (1./4.)*(surf_13_1 + surf_13_2 + surf_13_3 + surf_13_4)
surf_s22 = (1./4.)*(surf_22_1 + surf_22_2 + surf_22_3 + surf_22_4)
surf_s23 = (1./4.)*(surf_23_1 + surf_23_2 + surf_23_3 + surf_23_4)
surf_s33 = (1./4.)*(surf_33_1 + surf_33_2 + surf_33_3 + surf_33_4)

delete solution_X, solution_Y, solution_Z 
delete residual_X, residual_Y, residual_Z

delete CAUCHY_STRESS_01, CAUCHY_STRESS_10, CAUCHY_STRESS_19, CAUCHY_STRESS_28, CAUCHY_STRESS_37, CAUCHY_STRESS_46, > 
       CAUCHY_STRESS_55, CAUCHY_STRESS_64
delete CAUCHY_STRESS_02, CAUCHY_STRESS_11, CAUCHY_STRESS_20, CAUCHY_STRESS_29, CAUCHY_STRESS_38, CAUCHY_STRESS_47, >
       CAUCHY_STRESS_56, CAUCHY_STRESS_65
delete CAUCHY_STRESS_03, CAUCHY_STRESS_12, CAUCHY_STRESS_21, CAUCHY_STRESS_30, CAUCHY_STRESS_39, CAUCHY_STRESS_48, >
       CAUCHY_STRESS_57, CAUCHY_STRESS_66
delete CAUCHY_STRESS_04, CAUCHY_STRESS_13, CAUCHY_STRESS_22, CAUCHY_STRESS_31, CAUCHY_STRESS_40, CAUCHY_STRESS_49, >
       CAUCHY_STRESS_58, CAUCHY_STRESS_67
delete CAUCHY_STRESS_05, CAUCHY_STRESS_14, CAUCHY_STRESS_23, CAUCHY_STRESS_32, CAUCHY_STRESS_41, CAUCHY_STRESS_50, >
       CAUCHY_STRESS_59, CAUCHY_STRESS_68
delete CAUCHY_STRESS_06, CAUCHY_STRESS_15, CAUCHY_STRESS_24, CAUCHY_STRESS_33, CAUCHY_STRESS_42, CAUCHY_STRESS_51, >
       CAUCHY_STRESS_60, CAUCHY_STRESS_69
delete CAUCHY_STRESS_07, CAUCHY_STRESS_16, CAUCHY_STRESS_25, CAUCHY_STRESS_34, CAUCHY_STRESS_43, CAUCHY_STRESS_52, >
       CAUCHY_STRESS_61, CAUCHY_STRESS_70
delete CAUCHY_STRESS_08, CAUCHY_STRESS_17, CAUCHY_STRESS_26, CAUCHY_STRESS_35, CAUCHY_STRESS_44, CAUCHY_STRESS_53, >
       CAUCHY_STRESS_62, CAUCHY_STRESS_71
delete CAUCHY_STRESS_09, CAUCHY_STRESS_18, CAUCHY_STRESS_27, CAUCHY_STRESS_36, CAUCHY_STRESS_45, CAUCHY_STRESS_54, >
       CAUCHY_STRESS_63, CAUCHY_STRESS_72

delete SURF_CAUCHY_STRESS_01, SURF_CAUCHY_STRESS_02, SURF_CAUCHY_STRESS_03, SURF_CAUCHY_STRESS_04, >
       SURF_CAUCHY_STRESS_05, SURF_CAUCHY_STRESS_06, SURF_CAUCHY_STRESS_07, SURF_CAUCHY_STRESS_08
delete SURF_CAUCHY_STRESS_09, SURF_CAUCHY_STRESS_10, SURF_CAUCHY_STRESS_11, SURF_CAUCHY_STRESS_12, >
       SURF_CAUCHY_STRESS_13, SURF_CAUCHY_STRESS_14, SURF_CAUCHY_STRESS_15, SURF_CAUCHY_STRESS_16
delete SURF_CAUCHY_STRESS_17, SURF_CAUCHY_STRESS_18, SURF_CAUCHY_STRESS_19, SURF_CAUCHY_STRESS_20, >
       SURF_CAUCHY_STRESS_21, SURF_CAUCHY_STRESS_22, SURF_CAUCHY_STRESS_23, SURF_CAUCHY_STRESS_24
delete SURF_CAUCHY_STRESS_25, SURF_CAUCHY_STRESS_26, SURF_CAUCHY_STRESS_27, SURF_CAUCHY_STRESS_28, >
       SURF_CAUCHY_STRESS_29, SURF_CAUCHY_STRESS_30, SURF_CAUCHY_STRESS_31, SURF_CAUCHY_STRESS_32
delete SURF_CAUCHY_STRESS_33, SURF_CAUCHY_STRESS_34, SURF_CAUCHY_STRESS_35, SURF_CAUCHY_STRESS_36

exit
