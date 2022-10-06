#!/usr/bin/python3

import numpy as np
import pandas as pd
import os
from bokeh.io import output_file, show
from bokeh.plotting import figure
from bokeh.layouts import column, row
from bokeh.palettes import Dark2_6 as palette
from bokeh.models.widgets import Div
from bokeh.models import ColumnDataSource
import itertools
import argparse

# Constant things
callback_log_suffix = "_callback_log.csv"
file_log_suffix = "_file_log.csv"
file_prefix_length = 14 # length of the pointer prefix

# sfizz 0.3.0 logs
callback_log_columns = ['Dispatch', 'RenderMethod', 'Data', 'Amplitude', 'Filters', 'Panning', 'Effects', 'NumVoices', 'NumSamples']
file_log_columns = ['WaitDuration', 'LoadDuration', 'FileSize', 'FileName']

# Helper functions
def scale_columns(dataframe, column_list, scale_factor):
    """Scale the columns of a pandas Dataframe

    Arguments:
        dataframe {pandas Dataframe} -- a dataframe
        column_list {list of strings} -- the list of column names to scale
        scale_factor {arithmetic type} -- the scaling factor
    """
    for column in column_list:
        dataframe[column] *= scale_factor

def html_list(string_list):
    """Returns an HTML list from a list of strings

    Arguments:
        string_list {list of strings} -- the input list

    Returns:
        string -- the list of string formatted as an HTML list
    """
    returned_string = "<ul>"
    for string in string_list:
        returned_string += f"<li>{string}</li>"
    returned_string += "</ul>"
    return returned_string

def print_summary_to_console(title, lines):
    """Prints a multi-line summary to the console

    Arguments:
        title {string} -- The summary title
        lines {list of strings} -- The summary lines
    """
    print(title)
    print('- ', end='')
    print('\n- '.join(lines))
    print('\n')

def extract_file_name_and_prefix(file_name):
    """From a file name formatted as "0xAE152342334152_sfzFileName_...", extract the sfz file name and the pointer prefix

    Arguments:
        file_name {string} -- The mangled sfizz log filename

    Returns:
        (string, string) -- File name and file prefix
    """
    file_prefix = file_name[:file_prefix_length]

    if file_name.endswith(file_log_suffix):
        suffix_length = len(file_log_suffix)
    elif file_name.endswith(callback_log_suffix):
        suffix_length = len(callback_log_suffix)
    else:
        suffix_length = 0

    sfz_file_name = file_name[file_prefix_length + 1:-suffix_length]
    if sfz_file_name == '':
        sfz_file_name = "Empty filename"

    return sfz_file_name, file_prefix

def set_axis_and_legend(figure, xlabel=None, ylabel=None, hide_on_click=True):
    """Generic way to set the axis labels and enable clicking on the legend to hide the plot

    Arguments:
        figure {Bokeh figure}

    Keyword Arguments:
        xlabel {string} -- the x label (default: {None})
        ylabel {string} -- the y label (default: {None})
        hide_on_click {bool} -- whether to hide the legend when clicking (default: {True})
    """
    if xlabel is not None:
        figure.xaxis.axis_label = xlabel
    if ylabel is not None:
        figure.yaxis.axis_label = ylabel
    if hide_on_click:
        figure.legend.click_policy = "hide"

# Argument parser
parser = argparse.ArgumentParser(description="Plot performance summary and generate a detailed report on sfizz's performance")
parser.add_argument("files", nargs="+", type=str, help="The csv log files to consider")
parser.add_argument("--output", type=str, default="report.html", help="The detailed output report file name")
parser.add_argument("--title", type=str, default="sfizz's performance report", help="The report title")
parser.add_argument("-v", "--verbose", action='store_true', help="Verbose console output")
args = parser.parse_args()

# Check that all input files are here
for file in args.files:
    assert os.path.exists(file), f'Cannot find {file}'

if args.verbose:
    print(f'Input files:', args.files)
    print(f'Output file:', args.output)

output_file(args.output, args.title)

# Dispatch files into their respective lists
file_log_list = [file for file in args.files if file.endswith(file_log_suffix)]
callback_log_list = [file for file in args.files if file.endswith(callback_log_suffix)]

# Plot the render duration and number of voices for all callback files
fig_num_voices = figure(plot_width=600, plot_height=400, title="Number of voices")
fig_callback_duration = figure(plot_width=600, plot_height=400, title="Render method")
colors = itertools.cycle(palette)
for file_name in callback_log_list:
    sfz_file_name, file_prefix = extract_file_name_and_prefix(file_name)
    csv_data = pd.read_csv(file_name)
    assert (csv_data.columns == callback_log_columns).all(), f"Column mismatch for {file_name}"

    color = next(colors)
    fig_num_voices.line(csv_data.index, csv_data['NumVoices'], legend_label=f"{sfz_file_name} ({file_prefix[-4:]})", color=color)
    fig_callback_duration.line(csv_data.index, csv_data['RenderMethod'] * 1e6, legend_label=f"{sfz_file_name} ({file_prefix[-4:]})", color=color)
set_axis_and_legend(fig_num_voices, 'Callback index', 'Number of voices')
set_axis_and_legend(fig_callback_duration, 'Callback index', 'Callback duration (µs)')

# Callback breakdowns plots per file
callback_figures = []
for file_name in callback_log_list:
    file_prefix = file_name[:file_prefix_length]
    sfz_file_name = file_name[file_prefix_length + 1:-len(callback_log_suffix)]
    if sfz_file_name == '':
        sfz_file_name = "Empty filename"
    csv_data = pd.read_csv(file_name)

    # Scale the data and add some columns
    scale_columns(csv_data, ['Dispatch', 'RenderMethod', 'Data', 'Amplitude', 'Panning', 'Filters', 'Effects'], 1e6)
    csv_data['DataPerVoice'] = csv_data['Data'] / csv_data['NumVoices']
    csv_data['AmplitudePerVoice'] = csv_data['Amplitude'] / csv_data['NumVoices']
    csv_data['FiltersPerVoice'] = csv_data['Filters'] / csv_data['NumVoices']
    csv_data['PanningPerVoice'] = csv_data['Panning'] / csv_data['NumVoices']
    csv_data['EffectsPerVoice'] = csv_data['Effects'] / csv_data['NumVoices']
    csv_data['Residual'] = (csv_data['RenderMethod'] - csv_data['Panning'] - csv_data['Filters'] - csv_data['Amplitude'] - csv_data['Data'] - csv_data['Effects']) / csv_data['NumVoices']

    # Prep the summary
    summary_title = f"Callback statistics summary for {sfz_file_name} ({file_prefix[-4:]})"
    summary_lines = [
        f"Samples per callback (avg/max): {csv_data['NumSamples'].mean():.1f}/{csv_data['NumSamples'].max()}",
        f"Active voices (avg/max): {csv_data['NumVoices'].mean():.1f}/{csv_data['NumVoices'].max()}",
        f"Dispatch duration (avg/max): {csv_data['Dispatch'].mean():.2f}/{csv_data['Dispatch'].max():.2f} µs",
        f"Render duration (avg/max): {csv_data['RenderMethod'].mean():.2f}/{csv_data['RenderMethod'].max():.2f} µs",
        f"Source data reading/generation (avg/max): {csv_data['Data'].mean():.2f}/{csv_data['Data'].max():.2f} µs",
        f"Amplitude processing (avg/max): {csv_data['Amplitude'].mean():.2f}/{csv_data['Amplitude'].max():.2f} µs",
        f"Panning processing (avg/max): {csv_data['Panning'].mean():.2f}/{csv_data['Panning'].max():.2f} µs",
        f"Filter processing (avg/max): {csv_data['Filters'].mean():.2f}/{csv_data['Filters'].max():.2f} µs",
        f"Effect processing (avg/max): {csv_data['Effects'].mean():.2f}/{csv_data['Effects'].max():.2f} µs"
    ]
    callback_figures.append(Div(text=f"<h3>{summary_title}</h3>" + html_list(summary_lines), width=600))
    if args.verbose:
        print_summary_to_console(summary_title, summary_lines)

    # Callback breakdown figure
    stacked_column_names = ['DataPerVoice', 'AmplitudePerVoice', 'FiltersPerVoice', 'PanningPerVoice', 'EffectsPerVoice', 'Residual']
    stacked_column_legends = ['Data', 'Amplitude', 'Filters', 'Panning', 'Effects', 'Residual']
    source = ColumnDataSource(csv_data)
    source.add(csv_data.index, 'index')

    fig_breakdown = figure(plot_width=600, plot_height=400, title=f"{sfz_file_name} - Callback breakdown")
    fig_breakdown.varea_stack(stacked_column_names, x='index', source=source, legend_label=stacked_column_legends, color=palette[:6])
    set_axis_and_legend(fig_breakdown, 'Callback index', 'Aggregate duration (per voice, average, µs)')

    # Breakdown histogram figure
    fig_histogram = figure(plot_width=600, plot_height=400, title=f"{sfz_file_name} - Callback breakdown histogram")
    histogram_bins = np.linspace(0, 10, 300)
    for idx, (column_name, legend_label) in enumerate(zip(stacked_column_names, stacked_column_legends)):
        bins, edges = np.histogram(csv_data[column_name], bins=histogram_bins, density=True)
        fig_histogram.quad(bottom=0, top=bins, left=edges[:-1], right=edges[1:], legend_label=legend_label, alpha=0.5, color=palette[idx])
    set_axis_and_legend(fig_histogram, 'Processing duration (per voice, average, µs)')

    # Add a row to the report
    callback_figures.append(row(fig_breakdown, fig_histogram))

# File timing plots
file_figures = []
for file_name in file_log_list:
    sfz_file_name, file_prefix = extract_file_name_and_prefix(file_name)
    csv_data = pd.read_csv(file_name)
    assert (csv_data.columns == file_log_columns).all(), f"Column mismatch for {file_name}"
    scale_columns(csv_data, ['WaitDuration', 'LoadDuration'], 1e6)
    normalized_load_duration = csv_data['LoadDuration'] / csv_data['FileSize']

    # Prep and print the summary
    summary_title = f"File loading statistics summary for {sfz_file_name} ({file_prefix[-4:]})"
    summary_lines = [
        f"Waiting duration (avg/max): {csv_data['WaitDuration'].mean():.2f}/{csv_data['WaitDuration'].max():.2f} µs",
        f"Loading duration (avg/max): {csv_data['LoadDuration'].mean():.2f}/{csv_data['LoadDuration'].max():.2f} µs",
        f"Normalized loading duration (avg/max): {normalized_load_duration.mean():.5f}/{normalized_load_duration.max():.5f} µs"
    ]
    file_figures.append(Div(text=f"<h3>{summary_title}</h3>" + html_list(summary_lines), width=600))
    if args.verbose:
        print_summary_to_console(summary_title, summary_lines)

    # Split the loading duration depending on the file extension
    norm_load_times = {}
    load_times = {}
    for idx, csv_row in csv_data.iterrows():
        file_extension = csv_row['FileName'].split('.')[-1]
        if file_extension not in load_times:
            load_times[file_extension] = []
        if file_extension not in norm_load_times:
            norm_load_times[file_extension] = []
        norm_load_times[file_extension].append(csv_row['LoadDuration'] / csv_row['FileSize'])
        load_times[file_extension].append(csv_row['LoadDuration'])

    # Waiting time histogram
    fig_wait_times = figure(plot_width=400, plot_height=400, title=f"{sfz_file_name} - Wait times")
    hist_wait, edges_wait = np.histogram(csv_data['WaitDuration'], bins=100, density=True)
    fig_wait_times.quad(top=hist_wait, bottom=0, left=edges_wait[:-1], right=edges_wait[1:], fill_color=palette[0], alpha=0.5)
    set_axis_and_legend(fig_wait_times, 'Wait time (µs)', hide_on_click=False)

    # Normalized load time histogram
    colors = itertools.cycle(palette)
    fig_norm_load_times = figure(plot_width=400, plot_height=400, title=f"{sfz_file_name} - Normalized load times")
    for extension in load_times:
        hist, edges = np.histogram(np.array(norm_load_times[extension]), bins=100, density=True)
        fig_norm_load_times.quad(top=hist, bottom=0, left=edges[:-1], right=edges[1:],
            fill_color=next(colors), alpha=0.5, legend_label=extension)
    set_axis_and_legend(fig_norm_load_times, 'Load time per sample (µs)')

    # Load time histogram
    colors = itertools.cycle(palette)
    fig_load_times = figure(plot_width=400, plot_height=400, title=f"{sfz_file_name} - Load times")
    for extension in load_times:
        hist, edges = np.histogram(np.array(load_times[extension]), bins=100, density=True)
        fig_load_times.quad(top=hist, bottom=0, left=edges[:-1], right=edges[1:],
               fill_color=next(colors), alpha=0.5, legend_label=extension)
    set_axis_and_legend(fig_load_times, 'Load time (µs)')

    # Add a row to the report
    file_figures.append(row(fig_wait_times, fig_load_times, fig_norm_load_times))

# Show the output
show(column(
    Div(text=f"<h1>{args.title}</h1> Input files: {html_list(args.files)}"),
    row(fig_num_voices, fig_callback_duration),
    *callback_figures,
    *file_figures
))
