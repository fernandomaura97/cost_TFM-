

format long 
x1 = 0:0.000001:0.04;

x2 = 0.04:0.000001:0.08;

x3 = 0.08:0.000001:0.12;

x4 = 0.12:0.000001:0.3;

y1 = 1;
y2 = -5*x2 + 1.2;
y3 = -15*x3 + 2;
y4 = -1.11111*x4 + 0.333333;


plot(x1,y1 , 'b');
hold on
plot(x2,y2, 'b');
plot(x3,y3, 'b');
plot(x4,y4, 'b');
grid on 
hold off
shg


