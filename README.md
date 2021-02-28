#  Which stories appear both on Lobsters and on HN, who was first?

**[See here for more information](https://raymii.org/s/software/Cpp_exercise_in_parsing_json_http_apis_and_time_stuff.html).**

## Installation and usage

Usual cmake steps, dependency on OpenSSL (`apt install libssl-dev`) 

Clone the repository:

    git clone https://github.com/RaymiiOrg/lobsters-hn-post-compare
    cd lobsters-hn-post-compare

Setup a cmake folder:

    mkdir build
    cd build
    cmake ..

Compile:

    make 

Binary `hn_lob_comp` should be in the same folder.

### Usage

    chmod +x ./hn_lob_comp
    ./hn_lob_comp help


Output:

    Which stories appear both on Lobsters and on HN, who was first?
    An excuse to play with parsing a JSON api in C++ with async by Remy van Elst (https://raymii.org)
    
    Current date/time: 2020-12-30T22:21:43 +0100
    
    Usage: ./hn_lob_comp [help|test|top|new]
    ./hn_lob_comp top: analyze top stories from HN & Lobsters.
    ./hn_lob_comp help: this text.
    ./hn_lob_comp test: run a test to check your timezones.
    ./hn_lob_comp new: get new posts instead of best.

You'll probably want the `top` command:

    ./hn_lob_comp top

## Output 

Here's what a `top` run looks like:


```

Which stories appear both on Lobsters and on HN, who was first?
An excuse to play with parsing a JSON api in C++ with async by Remy van Elst (https://raymii.org)

Current date/time: 2020-12-30T22:24:57 +0100

Fetching HackerNews Best Stories async (200 posts) (https://github.com/HackerNews/API)
Fetching the first ten Lobsters pages async 10*25=200 posts) (https://lobste.rs/s/r9oskz/is_there_api_documentation_for_lobsters_somewhere)

Number of posts from Lobsters    : 200
Number of posts from Hacker News : 192

Matches (13):

# ACE: Apple Type-C Port Controller Secrets  
URL: https://blog.t8012.dev/ace-part-1/  
First appeared on **HackerNews** with 257 votes and 106 comments, submitted by aunali1 (2020-12-30T07:49:16 +0100; https://news.ycombinator.com/item?id=25579286 ).  
After 6 hours, 19 minutes, 28 seconds it was submitted to **Lobsters** by calvin with 9 votes and 1 comments (2020-12-30T14:08:44 +0100; https://lobste.rs/s/so3rb4/ace_apple_type_c_port_controller_secrets ).  
The highest score was reached on HackerNews and the most comments were on HackerNews.  

# Computer Science textbooks that are freely available online  
URL: https://csgordon.github.io/books.html  
First appeared on **HackerNews** with 534 votes and 50 comments, submitted by MrXOR (2020-12-29T19:14:07 +0100; https://news.ycombinator.com/item?id=25572852 ).  
After 15 hours, 48 minutes, 53 seconds it was submitted to **Lobsters** by redecas with 10 votes and 0 comments (2020-12-30T11:03:00 +0100; https://lobste.rs/s/blu8jq/colin_s_gordon_reading_list ).  
The highest score was reached on HackerNews and the most comments were on HackerNews.  

# Against Essential and Accidental Complexity  
URL: https://danluu.com/essential-complexity/  
First appeared on **HackerNews** with 239 votes and 91 comments, submitted by weinzierl (2020-12-29T13:37:22 +0100; https://news.ycombinator.com/item?id=25569148 ).  
After 1 hours, 18 minutes, 24 seconds it was submitted to **Lobsters** by j11g with 40 votes and 8 comments (2020-12-29T14:55:46 +0100; https://lobste.rs/s/gvahe2/against_essential_accidental ).  
The highest score was reached on HackerNews and the most comments were on HackerNews.  

# C Template Library  
URL: https://github.com/glouw/ctl  
First appeared on **Lobsters** with 39 votes and 11 comments, submitted by glouwbug (2020-12-30T00:25:37 +0100; https://lobste.rs/s/9gc2ku/c_template_library ).  
**Within the hour this was also posted to HackerNews!**
After 14 minutes, 54 seconds it was submitted to **HackerNews** by glouwbug with 243 votes and 115 comments (2020-12-30T00:40:31 +0100; https://news.ycombinator.com/item?id=25576466 ).  
The highest score was reached on HackerNews and the most comments were on HackerNews.  
**The same username submitted the post to both sites**.  

# Niex: Jupyter Notebooks but Using Elixir  
URL: https://github.com/jonklein/niex  
First appeared on **HackerNews** with 128 votes and 21 comments, submitted by niels_bom (2020-12-28T23:20:20 +0100; https://news.ycombinator.com/item?id=25563935 ).  
**Within the hour this was also posted to Lobsters!**
After 2 minutes, 30 seconds it was submitted to **Lobsters** by friendlysock with 10 votes and 0 comments (2020-12-28T23:22:50 +0100; https://lobste.rs/s/dnw8g7/jonklein_niex_interactive_elixir_code ).  
The highest score was reached on HackerNews and the most comments were on HackerNews.  

# Cosmopolitan Libc: build-once run-anywhere C library  
URL: https://justine.lol/cosmopolitan/index.html  
First appeared on **HackerNews** with 589 votes and 163 comments, submitted by pantalaimon (2020-12-28T03:59:32 +0100; https://news.ycombinator.com/item?id=25556286 ).  
After 6 hours, 3 minutes, 10 seconds it was submitted to **Lobsters** by GrayGnome with 112 votes and 15 comments (2020-12-28T10:02:42 +0100; https://lobste.rs/s/xnqpyp/cosmopolitan_c_library ).  
The highest score was reached on HackerNews and the most comments were on HackerNews.  

# Virtualize Your Network on FreeBSD with VNET  
URL: https://klarasystems.com/articles/virtualize-your-network-on-freebsd-with-vnet/  
First appeared on **HackerNews** with 74 votes and 12 comments, submitted by vermaden (2020-12-30T11:00:09 +0100; https://news.ycombinator.com/item?id=25580286 ).  
**Within the hour this was also posted to Lobsters!**
After 12 seconds it was submitted to **Lobsters** by vermaden with 2 votes and 0 comments (2020-12-30T11:00:21 +0100; https://lobste.rs/s/r5bhmo/virtualize_your_network_on_freebsd_with ).  
The highest score was reached on HackerNews and the most comments were on HackerNews.  
**The same username submitted the post to both sites**.  

# Why the iPhone Timer app displays a fake time  
URL: https://lukashermann.dev/writing/why-the-iphone-timer-displays-fake-time/  
First appeared on **HackerNews** with 220 votes and 74 comments, submitted by _antix (2020-12-28T22:56:41 +0100; https://news.ycombinator.com/item?id=25563708 ).  
After 13 hours, 28 minutes, 51 seconds it was submitted to **Lobsters** by Tenzer with 51 votes and 19 comments (2020-12-29T12:25:32 +0100; https://lobste.rs/s/yvw2xg/why_iphone_timer_app_displays_fake_time ).  
The highest score was reached on HackerNews and the most comments were on HackerNews.  

# Contributing Without Code  
URL: https://popey.com/blog/2020/12/contributing-without-code/  
First appeared on **HackerNews** with 109 votes and 22 comments, submitted by pabs3 (2020-12-30T03:36:42 +0100; https://news.ycombinator.com/item?id=25577746 ).  
After 9 hours, 48 minutes, 21 seconds it was submitted to **Lobsters** by learnbyexample with 1 votes and 0 comments (2020-12-30T13:25:03 +0100; https://lobste.rs/s/ms9h3i/contributing_without_code ).  
The highest score was reached on HackerNews and the most comments were on HackerNews.  

# Getting Started in BBC Basic  
URL: https://www.bbcmicrobot.com/learn/index.html  
First appeared on **HackerNews** with 68 votes and 25 comments, submitted by ingve (2020-12-29T13:36:33 +0100; https://news.ycombinator.com/item?id=25569146 ).  
After 1 days, 6 hours, 50 minutes, 23 seconds it was submitted to **Lobsters** by gerikson with 3 votes and 0 comments (2020-12-30T20:26:56 +0100; https://lobste.rs/s/fo82h4/getting_started_bbc_basic ).  
The highest score was reached on HackerNews and the most comments were on HackerNews.  

# Implementing join planning in our open source Golang SQL query engine  
URL: https://www.dolthub.com/blog/2020-12-28-join-planning/  
First appeared on **HackerNews** with 100 votes and 15 comments, submitted by zachmu (2020-12-28T18:39:24 +0100; https://news.ycombinator.com/item?id=25561173 ).  
After 21 hours, 21 minutes, 7 seconds it was submitted to **Lobsters** by eatonphil with 3 votes and 0 comments (2020-12-29T16:00:31 +0100; https://lobste.rs/s/a6suos/planning_joins_make_use_indexes ).  
The highest score was reached on HackerNews and the most comments were on HackerNews.  

# Teaching the Unfortunate Parts  
URL: https://www.executeprogram.com/blog/teaching-the-unfortunate-parts  
First appeared on **HackerNews** with 123 votes and 90 comments, submitted by gary_bernhardt (2020-12-29T00:37:26 +0100; https://news.ycombinator.com/item?id=25564666 ).  
After 7 hours, 22 minutes, 49 seconds it was submitted to **Lobsters** by Hail_Spacecake with 9 votes and 6 comments (2020-12-29T08:00:15 +0100; https://lobste.rs/s/4ptsq9/teaching_unfortunate_parts ).  
The highest score was reached on HackerNews and the most comments were on HackerNews.  

# Running BSDs on AMD Ryzen 5000 Series – FreeBSD/Linux Benchmarks  
URL: https://www.phoronix.com/scan.php?page=article&item=amd-5900x-bsd  
First appeared on **Lobsters** with 3 votes and 0 comments, submitted by vermaden (2020-12-30T11:02:18 +0100; https://lobste.rs/s/fmooqn/running_bsds_on_amd_ryzen_5000_series ).  
**Within the hour this was also posted to HackerNews!**
After 5 seconds it was submitted to **HackerNews** by vermaden with 77 votes and 42 comments (2020-12-30T11:02:23 +0100; https://news.ycombinator.com/item?id=25580298 ).  
The highest score was reached on HackerNews and the most comments were on HackerNews.  
**The same username submitted the post to both sites**.  

# Bash HTTP Monitoring Dashboard
URL: https://raymii.org/s/software/Bash_HTTP_Monitoring_Dashboard.html
First appeared on **Lobsters** with 30 votes and 2 comments, submitted by raymii (2020-12-27T13:58:40 +0100; https://lobste.rs/s/4pivy1/bash_http_monitoring_dashboard ).
**Within the hour this was also posted to HackerNews!**  
After 5 minutes, 36 seconds it was submitted to **HackerNews** by todsacerdoti with 160 votes and 26 comments (2020-12-27T14:04:16 +0100; https://news.ycombinator.com/item?id=25550732 ).  
The highest score was reached on HackerNews and the most comments were on HackerNews.

3 posts appeared first on Lobsters and 11 posts appeared first on HackerNews.
Average time for a cross-post: 8 hours, 39 minutes, 55 seconds .
Average comments on HN: 64, Lobsters: 4.
Average score on HN: 212, Lobsters: 22. 

```

    
