#pragma once

#define _USE_FIXED_RAND

template <class type_t>
class neuron
{
protected:
	static const int RANK = 2;

public:
	neuron(int input, int output);
	~neuron();

	void fwd(concurrency::array_view<type_t, RANK> &ar_x);
	void bkwd(concurrency::array_view<type_t, RANK> &ar_delta_in);
	void accm(concurrency::array_view<type_t, RANK> &ar_x);

	concurrency::array_view<type_t, 2>& get_ar_y();
	concurrency::array_view<type_t, 2>& get_ar_delta();
	concurrency::array_view<type_t, 2>& get_ar_W();
	concurrency::array_view<type_t, 2>& get_ar_error();

	void updt(int samples);
	void set_error();

protected:
	type_t alpha = 1;
	const int COLS = 1;

	std::random_device rd;

	int x_rows;
	int x_cols;

	int t_x_rows;
	int t_x_cols;
	std::vector<type_t> t_x;

	int W_rows;
	int W_cols;
	int W_size;
	std::vector<type_t> W;

	int W_Trans_rows;
	int W_Trans_cols;
	std::vector<type_t> W_Trans;

	int z_rows;
	int z_cols;
	std::vector<type_t> z;

	int y_rows;
	int y_cols;
	std::vector<type_t> y;

	int t_y_rows;
	int t_y_cols;
	std::vector<type_t> t_y;

	int t_e0_rows;
	int t_e0_cols;
	std::vector<type_t> t_e0;

	int error_rows;
	int error_cols;
	std::vector<type_t> error;

	int ones_rows;
	int ones_cols;
	std::vector<type_t> ones;

	int delta_W_rows;
	int delta_W_cols;
	std::vector<type_t> delta_W;

	int t_delta_W_rows;
	int t_delta_W_cols;
	std::vector<type_t> t_delta_W;

	int delta_rows;
	int delta_cols;
	std::vector<type_t> delta;

	int t_delta_out_rows;
	int t_delta_out_cols;
	std::vector<type_t> t_delta_out;

	concurrency::array_view<type_t, RANK> ar_y;
	concurrency::array_view<type_t, RANK> ar_delta;
	concurrency::array_view<type_t, RANK> ar_W;
	concurrency::array_view<type_t, RANK> ar_error;
	concurrency::array_view<type_t, RANK> ar_z;
	concurrency::array_view<type_t, RANK> ar_t_e0;
	concurrency::array_view<type_t, RANK> ar_t_x;
	concurrency::array_view<type_t, RANK> ar_t_delta_out;
	concurrency::array_view<type_t, RANK> ar_delta_W;
	concurrency::array_view<type_t, RANK> ar_t_delta_W;
	concurrency::array_view<type_t, RANK> ar_W_Trans;
	concurrency::array_view<type_t, RANK> ar_t_y;
	concurrency::array_view<type_t, RANK> ar_ones;

	virtual void activate() = 0;
	virtual void activate_der() = 0;
	
	void init_rand(int size, std::vector<type_t> &vect);
	void updateAlpha();
};

template <class type_t>
void neuron<type_t>::updateAlpha()
{
	const auto CONST1 = 1;
	const auto CONST2 = 1;
	
	alpha = CONST1 / (alpha + CONST2);
}

template <class type_t>
neuron<type_t>::neuron(int input, int output) :
	y(input * COLS, 0),
	delta(input * COLS, 0),
	W(input * output, 1),
	error(output * COLS, 0),
	z(input * COLS, 0),
	t_e0(input * COLS, 0),
	t_x(output * COLS, 0),
	t_delta_out(input * output, 0),
	delta_W(input * output, 0),
	t_delta_W(input * output, 0),
	W_Trans(output * input, 0),
	t_y(input * COLS, 0),
	ones(input * COLS, 1),

	ar_y(input, COLS, y),
	ar_delta(input, COLS, delta),
	ar_W(input, output, W),
	ar_error(output, COLS, error),
	ar_z(input, COLS, z),
	ar_t_e0(input, COLS, t_e0),
	ar_t_x(output, COLS, t_x),
	ar_t_delta_out(input, output, t_delta_out),
	ar_delta_W(input, output, delta_W),
	ar_t_delta_W(input, output, t_delta_W),
	ar_W_Trans(output, input, W_Trans),
	ar_t_y(input, COLS, t_y),
	ar_ones(input, COLS, ones)
{
	x_rows = output;
	x_cols = COLS;

	t_x_rows = output;
	t_x_cols = COLS;

	W_rows = input;
	W_cols = output;

	W_Trans_rows = output;
	W_Trans_cols = input;

	z_rows = input;
	z_cols = COLS;

	y_rows = input;
	y_cols = COLS;

	t_y_rows = input;
	t_y_cols = COLS;

	t_e0_rows = input;
	t_e0_cols = COLS;

	error_rows = output;
	error_cols = COLS;

	ones_rows = input;
	ones_cols = COLS;

	delta_W_rows = input;
	delta_W_cols = output;

	t_delta_W_rows = input;
	t_delta_W_cols = output;

	delta_rows = input;
	delta_cols = COLS;

	t_delta_out_rows = input;
	t_delta_out_cols = output;

	W_size = W_rows * W_cols;
	W.clear();
	init_rand(W_size, W);
	ar_W.synchronize();
}

template <class type_t>
neuron<type_t>::~neuron()
{
}

template <class type_t>
void neuron<type_t>::fwd(concurrency::array_view<type_t, RANK> &ar_x)
{
	nnet_math<type_t>::matrix_mult(ar_W, ar_x, ar_z);

	activate();
}

template <class type_t>
void neuron<type_t>::bkwd(concurrency::array_view<type_t, RANK> &ar_delta_in)
{
	activate_der();

	nnet_math<type_t>::matrix_prod(ar_delta_in, ar_t_e0, ar_delta);
}

template <class type_t>
void neuron<type_t>::accm(concurrency::array_view<type_t, RANK> &ar_x)
{
	nnet_math<type_t>::matrix_trans(ar_x, ar_t_x);

	nnet_math<type_t>::matrix_mult(ar_delta, ar_t_x, ar_t_delta_out);

	nnet_math<type_t>::matrix_add(ar_t_delta_out, ar_delta_W, ar_delta_W);
}

template <class type_t>
void neuron<type_t>::updt(int samples)
{
	nnet_math<type_t>::scalar_div(ar_delta_W, samples, ar_t_delta_W);
	nnet_math<type_t>::scalar_mult(ar_t_delta_W, alpha, ar_t_delta_W);
	nnet_math<type_t>::matrix_sub(ar_W, ar_t_delta_W, ar_W);

	/* Update alpha */
	updateAlpha();
}

template <class type_t>
void neuron<type_t>::set_error()
{
	nnet_math<type_t>::matrix_trans(ar_W, ar_W_Trans);
	nnet_math<type_t>::matrix_mult(ar_W_Trans, ar_delta, ar_error);
}

template <class type_t>
concurrency::array_view<type_t, 2>& neuron<type_t>::get_ar_y()
{
	return ar_y;
}

template <class type_t>
concurrency::array_view<type_t, 2>& neuron<type_t>::get_ar_delta()
{
	return ar_delta;
}

template <class type_t>
concurrency::array_view<type_t, 2>& neuron<type_t>::get_ar_W()
{
	return ar_W;
}

template <class type_t>
concurrency::array_view<type_t, 2>& neuron<type_t>::get_ar_error()
{
	return ar_error;
}

template<class type_t>
inline void neuron<type_t>::init_rand(int size, std::vector<type_t>& vect)
{
#ifdef _USE_FIXED_RAND
	/* Always generate the same random numbers */
	std::mt19937 gen(1);
#else
	/* Always generate different random numbers */
	std::mt19937 gen(rd());
#endif

	std::uniform_real_distribution<type_t> distReal(-1, 1);
	const type_t sigma = sqrt((type_t)2 / size);
	std::normal_distribution<type_t> distNorm(0, sigma);

	for (int i = 0; i < size; i++)
	{
		vect.push_back(distReal(gen));
	}
}
