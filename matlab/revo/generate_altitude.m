% Generate the symbolic code for the kalman filter on altitude

dT = sym('dT','real');
A = [1 dT 0 0; 0 1 dT 0; 0 0 1 0; 0 0 0 1];
%Nu = diag([sym('V[1]') sym('V[2]') sym('V[3]') sym('V[4]')]);
%Nu = [sym('V[1][1]') 0              0              0; ...
%      0              sym('V[2][2]') 0              0; ...
%      0              0              sym('V[3][3]') sym('V[3][4]'); ...
%      0              0              sym('V[4][3]') sym('V[4][4]')];
Nu = [sym('V[1][1]') sym('V[1][2]') sym('V[1][3]') sym('V[1][4]'); ...
      sym('V[2][1]') sym('V[2][2]') sym('V[2][3]') sym('V[2][4]'); ...
      sym('V[3][1]') sym('V[3][2]') sym('V[3][3]') sym('V[3][4]'); ...
      sym('V[4][1]') sym('V[4][2]') sym('V[4][3]') sym('V[4][4]');];


Gamma = diag([sym('G[1]') sym('G[2]') sym('G[3]') sym('G[4]')]);
Sigma = diag([sym('S[1]') sym('S[2]')]);
C = [1 0 0 0; 0 0 1 1];
state = [sym('z[1]'); sym('z[2]'); sym('z[3]'); sym('z[4]')];
measure = [sym('x[1]'); sym('x[2]')];

P = simplify(A * Nu * A' + Gamma);
K = simplify(P*C'*(C*P*C'+Sigma)^-1);

% fill in the zeros from above equations to make next calculation sparse
P_mat = [sym('P[1][1]') sym('P[1][2]') sym('P[1][3]') sym('P[1][4]'); ...
         sym('P[2][1]') sym('P[2][2]') sym('P[2][3]') sym('P[2][4]'); ...
         sym('P[3][1]') sym('P[3][2]') sym('P[3][3]') sym('P[3][4]'); ...
         sym('P[4][1]') sym('P[4][3]') sym('P[4][3]') sym('P[4][4]')];
K_mat = [sym('K[1][1]') sym('K[1][2]'); ...
         sym('K[2][1]') sym('K[2][2]'); ...
         sym('K[3][1]') sym('K[3][2]'); ...
         sym('K[4][1]') sym('K[4][2]')];

z_new = A * state + K_mat * (measure - C * A * state);
V = (eye(4) - K_mat * C) * P_mat;
    
ccode(P)
ccode(K)
ccode(z_new)
ccode(V)


%% For when there is no baro update
% Generate the symbolic code for the kalman filter on altitude
C = [0 0 1 1];
Sigma = sym('S[2]');
measure = [sym('x[2]')];

P = simplify(A * Nu * A' + Gamma);
K = simplify(P*C'*(C*P*C'+Sigma)^-1);

% fill in the zeros from above equations to make next calculation sparse
P_mat = [sym('P[1][1]') sym('P[1][2]') sym('P[1][3]') sym('P[1][4]'); ...
         sym('P[2][1]') sym('P[2][2]') sym('P[2][3]') sym('P[2][4]'); ...
         sym('P[3][1]') sym('P[3][2]') sym('P[3][3]') sym('P[3][4]'); ...
         sym('P[4][1]') sym('P[4][3]') sym('P[4][3]') sym('P[4][4]')];
K_mat = [sym('K[1][1]'); ...
         sym('K[2][1]'); ...
         sym('K[3][1]'); ...
         sym('K[4][1]')];

z_new = A * state + K_mat * (measure - C * A * state);
V = (eye(4) - K_mat * C) * P_mat;
    
ccode(P)
ccode(K)
ccode(z_new)
ccode(V)
