#ifndef PID_H
#define PID_H

class PID_Controller {
  private:
    int _Ts, _r;
    int _min_lux, _max_lux;
    float _y, _e, _p, _i, _d, _aw, _u;
    float _y_prev, _e_prev, _i_prev, _d_prev;
    double _K, _K1, _K2, _K3, _K4;

    void _deadzone();
    void _saturate();

  public:
    // Constructor
    PID_Controller(int T, int r, double Kp, double Ki, double Kd, int a, int b);

    // Getters
    int getRef();
    int getMin();
    int getMax();

    // Setters
    void incRef(int v);
    void decRef(int v);
    void setRef(int r);
    void setPeriod(int T);
    void setGains(double Kp, double Ki, double Kd, int a, int b);

    // Commands
    void sample(float y);
    int process();
    void flush();
};

#endif
