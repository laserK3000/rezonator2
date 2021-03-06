Calculation of Hyper-Gaussian Beams
===================================

.. |MI| replace:: `M`\ :sup:`2`
.. |z0| replace:: `z`\ :sub:`0`
.. |w0| replace:: `w`\ :sub:`0`

Algorithm for calculation of hyper-Gaussian beams is applied to analyze single-pass systems (SP) when divergence parameter (|MI|) of the input beam set to a value larger than one.

.. index:: single: hyper-gaussian beam
.. index:: single: pseudo-gaussian beam

Hyper-Gaussian beam
-------------------

Hyper-Gaussian beam in |rezonator| is defined as a beam whose divergence is |MI| times greater than the divergence of a Gaussian beam having the same waist radius. Therefore its :ref:`Rayleigh distance<gauss_confocal>` is |MI| times less.

	.. image:: img/hypergauss_formula_1.png

.. index:: single: equivalent gaussian beam

Equivalent Gaussian beam
------------------------

For describing the propagation of hyper-Gaussian beam in free space, it is enough to replace the Rayleigh distance |z0| with |z0|/|MI| in formulas for `w(z)` and `R(z)`:

	.. image:: img/hypergauss_formula_2.png

It is equivalent to changing waist size from |w0| to |w0|/`M`: 

	.. image:: img/hypergauss_formula_3.png

A beam having such a reduced waist size is called *Equivalent Gaussian Beam*:

	.. image:: img/hypergauss_formula_4.png

Then we can express the waist radius of the hyper-Gaussian beam in terms of the equivalent Gaussian beam:

	.. image:: img/hypergauss_formula_5.png

And substitute it into formulas for `w(z)` and `R(z)`:

	.. image:: img/hypergauss_formula_6.png

Since equivalent Gaussian beam has the same Rayleigh distance as the hyper-Gaussian beam, its radius is `M` times less than the radius of the original beam not only at the waist but anywhere. 

	.. image:: img/hypergauss_formula_7.png

Because equivalent Gaussian beam is a standard Gaussian beam, its complex parameter can be transformed by ray matrices as usual.

Algorithm of computing
----------------------

The program uses the next algorithm for calculation of propagation of hyper-Gaussian beams: 

#. Equivalent size of the input beam is calculated. It is `M` times less than specified in the dialog of input beam parameters. 

#. Propagation of an equivalent Gaussian beam is calculated. 

#. Characteristics of the resulting hyper-Gaussian beam are calculated. Its radius is `M` times greater than the radius of the equivalent beam and radius of wavefront curvature is equal to that. 

	.. image:: ./img/hypergauss_algorithm.png

   
.. seealso::

    :doc:`gauss`
