#include "httplib.hpp"
#include "json.hpp"

#include <ctime>
#include <future>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <utility>
#include <vector>

#define CA_CERT_FILE "./ca-bundle.crt"

using json = nlohmann::json;

//wrapper function for checking whether a task has finished and
//the result can be retrieved by a std::future
template <typename R>
bool isReady(const std::future<R> &f)
{
    return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

struct Post
{
    friend std::ostream &operator<<(std::ostream &os, const Post &post)
    {

        os << "id: " << post.id << "; title: " << post.title << "; original_url: " << post.original_url << "; submitter: " << post.submitter << "; comment_url: " << post.comment_url << "; votes: " << post.votes << "; comment_count: " << post.comment_count << "; date UTC: " << printDateTimeUTC(post) << "; date local: " << printDateTimeLocal(post) << ";";
        return os;
    }
    static std::string printDateTimeLocal(const Post &post)
    {
        char _submit_date[200] {""};
        tm _localTime {0};
        _localTime = *localtime(&post.submit_timestamp);
        strftime(_submit_date, sizeof(_submit_date), "%Y-%m-%dT%H:%M:%S %z", &_localTime);
        return std::string(_submit_date);
    }
    static std::string printDateTimeUTC(const Post &post)
    {
        char _submit_date[200] {""};
        tm _utcTime {0};
        _utcTime = *gmtime(&post.submit_timestamp);
        strftime(_submit_date, sizeof(_submit_date), "%Y-%m-%dT%H:%M:%S %z", &_utcTime);
        return std::string(_submit_date);
    }
    [[nodiscard]] std::string printDateTimeUTC() const
    {
        return printDateTimeUTC(*this);
    }
    [[nodiscard]] std::string printDateTimeLocal() const
    {
        return printDateTimeLocal(*this);
    }
    std::string id;
    time_t submit_timestamp {0};
    std::string title;
    std::string original_url;
    std::string submitter;
    std::string comment_url;
    int votes {};
    int comment_count {};
    bool operator<(const Post &rhs) const
    {
        //        tm tm_lhs = submit_date;
        //        tm tm_rhs = rhs.submit_date;
        //        time_t t_lhs = mktime(&tm_lhs);
        //        time_t t_rhs = mktime(&tm_rhs);
        //        bool timeCmp = (t_lhs < t_rhs);
        return (original_url < rhs.original_url);
    }
    bool operator>(const Post &rhs) const
    {
        return rhs < *this;
    }
    bool operator<=(const Post &rhs) const
    {
        return !(rhs < *this);
    }
    bool operator>=(const Post &rhs) const
    {
        return !(*this < rhs);
    }
    bool operator==(const Post &rhs) const
    {
        return original_url == rhs.original_url;
    }
    bool operator!=(const Post &rhs) const
    {
        return !(rhs == *this);
    }
};

class httpException : public std::runtime_error
{
public:
    explicit httpException(const std::string &msg) :
        std::runtime_error(msg)
    {
    }
};

class aggregator
{
public:
    virtual std::vector<Post> parsePosts(nlohmann::json posts) = 0;
    virtual json getPosts() = 0;

    static json getJson(const std::string &domain, const std::string &url)
    {
        httplib::SSLClient cli(domain);
        cli.enable_server_certificate_verification(false);
        httplib::Headers headers = {};
        if (auto res = cli.Get(url.c_str()))
        {
            if (res->status != 200)
                throw httpException("HTTP Request failed. domain='" + domain + "', url='" + url + "', status code='" + std::to_string(res->status) + "', reason='" + res->reason + "'");

            auto result = json::parse(res->body);
            return result;
        }
        else
        {
            std::string sslError;
            if (auto result = cli.get_openssl_verify_result())
                sslError += X509_verify_cert_error_string(result);

            throw httpException("HTTP Request failed. domain='" + domain + "', url='" + url + "', httplib error='" + std::to_string((int)res.error()) + "', " + sslError);
        }
    }
};

class lobsters : public aggregator
{
public:
    explicit lobsters(std::string domain, std::string url) :
        _domain(std::move(domain)), _url(std::move(url)) {};
    std::vector<Post> parsePosts(json posts) override
    {
        std::vector<Post> result;
        for (const auto &page : posts)
        {
            for (const auto &item : page)
            {
                if (!item.contains("url"))
                    continue;

                Post p;
                if (item.contains("comment_count"))
                    p.comment_count = item["comment_count"];
                if (item.contains("comments_url"))
                    p.comment_url = item["comments_url"];
                if (item.contains("score"))
                    p.votes = item["score"];
                if (item.contains("title"))
                    p.title = item["title"];
                if (item.contains("url"))
                    p.original_url = item["url"];
                if (item.contains("short_id"))
                    p.id = item["short_id"];
                if (item.contains("created_at"))
                {
                    // format: 2020-12-28T00:22:26.000-06:00
                    std::string dateStr = item["created_at"];
                    // %z doesnt like the colon in the timezone
                    dateStr.erase(dateStr.begin() + 26);
                    struct tm cst
                    {
                        0
                    };
                    auto lobsters_convert = strptime(dateStr.c_str(), "%Y-%m-%dT%H:%M:%S.000%z", &cst);
                    if (lobsters_convert && lobsters_convert[0]) // strptime failed to convert
                        continue;

                    // timegm updates the static storage, copy it first.
                    auto lobsters_utc_offset = cst.tm_gmtoff; // gcc extension
                    time_t lobsters_epoch_without_timezone_offset = timegm(&cst); // epoch is in utc, so use timegm instead of mktime
                    time_t lobsters_epoch = difftime(lobsters_epoch_without_timezone_offset, lobsters_utc_offset);
                    p.submit_timestamp = lobsters_epoch;
                }

                if (item.contains("submitter_user"))
                    for (auto &[key, value] : item["submitter_user"].items())
                        if (key == "username")
                            p.submitter = value;

                result.push_back(p);
            }
        }
        return result;
    }

    json getPosts() override
    {
        json posts {};
        std::vector<std::future<json>> futures;
        int maxPages = 9;
        // Queue up all the items,
        for (int i = 1; i < maxPages; ++i)
        {
            std::string postUrl = std::regex_replace(_url, std::regex("%PAGENUMBER%"), std::to_string(i));
            futures.push_back(std::async(std::launch::async, getJson, _domain, postUrl));
        }

        // Wait until all futures are finished
        int finishedFutures = 1;
        while (finishedFutures < maxPages)
        {
            for (const auto &future : futures)
            {
                if (isReady(future))
                    ++finishedFutures;
            }
        }

        for (auto &future : futures)
        {
            posts.push_back(future.get());
        }

        return posts;
    }

private:
    std::string _url;
    std::string _domain;
};

class hackernews : public aggregator
{
public:
    explicit hackernews(std::string domain, std::string id_url, std::string story_url) :
        _domain(std::move(domain)), _id_url(std::move(id_url)), _story_url(std::move(story_url)) {};

    std::vector<Post> parsePosts(json posts) override
    {
        std::vector<Post> result;
        for (const auto &item : posts)
        {
            bool isStory = (item.contains("type") && item["type"].get<std::string>() == "story");
            if (!isStory)
                continue;
            if (!item.contains("url"))
                continue;

            Post p;
            if (item.contains("descendants"))
                p.comment_count = item["descendants"];
            if (item.contains("score"))
                p.votes = item["score"];
            if (item.contains("title"))
                p.title = item["title"];
            if (item.contains("url"))
                p.original_url = item["url"];
            if (item.contains("by"))
                p.submitter = item["by"];
            if (item.contains("id"))
            {
                p.id = std::to_string(item["id"].get<long long>());
                p.comment_url = "https://news.ycombinator.com/item?id=" + p.id;
            }
            if (item.contains("time"))
            {
                // format: 1609012592 (epoch) (epoch is always utc)
                std::string dateStr = std::to_string(item["time"].get<long long>());
                time_t hn_epoch = std::stoll(dateStr);
                p.submit_timestamp = hn_epoch;
            }

            result.push_back(p);
        }

        return result;
    }

    json getPosts() override
    {
        json posts {};
        std::vector<std::future<json>> futures;
        int counter = 1;
        int maxPosts = 200;

        // Queue up all the items,
        for (const auto &id : getJson(_domain, _id_url))
        {
            std::string postId = std::to_string(id.get<long long>());
            std::string postUrl = std::regex_replace(_story_url, std::regex("%ID%"), postId);
            futures.push_back(std::async(std::launch::async, getJson, _domain, postUrl));
            if (counter > maxPosts)
                break;
            ++counter;
        }

        // Wait until all futures are finished
        int finishedFutures = 0;
        while (finishedFutures < maxPosts)
        {
            for (const auto &future : futures)
            {
                if (isReady(future))
                    ++finishedFutures;
            }
        }

        for (auto &future : futures)
        {
            posts.push_back(future.get());
        }

        return posts;
    }

private:
    std::string _id_url;
    std::string _story_url;
    std::string _domain;
};

void printTm(const tm *tp);
void printCurrentDate()
{
    time_t t_now = time(nullptr);
    tm *tm_now = localtime(&t_now);
    char now[200] {""};
    strftime(now, sizeof(now), "%Y-%m-%dT%H:%M:%S %z", tm_now);
    std::cout << "Current date/time: " << now << "\n\n";
}

void printTm(const tm *tp)
{
    if (tp->tm_yday > 0)
        std::cout << tp->tm_yday << " days, ";
    if (tp->tm_hour > 0)
        std::cout << tp->tm_hour << " hours, ";
    if (tp->tm_min > 0)
        std::cout << tp->tm_min << " minutes, ";
    if (tp->tm_sec > 0)
        std::cout << tp->tm_sec << " seconds ";
}

template <typename T>
T calcAverage(const std::vector<T>& vec) {
    auto sum = std::accumulate(vec.cbegin(), vec.cend(), 0);
    return sum / vec.size();
}

void analyze(std::vector<Post> &lobstersPosts, std::vector<Post> &hnPosts)
{
    std::cout << "Number of posts from Lobsters    : " << lobstersPosts.size() << "\n";
    std::cout << "Number of posts from Hacker News : " << hnPosts.size() << "\n\n";

    // set_intersection requires vectors to be sorted
    std::sort(hnPosts.begin(), hnPosts.end());
    std::sort(lobstersPosts.begin(), lobstersPosts.end());

    std::vector<Post> post_intersection;
    std::set_intersection(hnPosts.begin(), hnPosts.end(), lobstersPosts.begin(), lobstersPosts.end(), std::back_inserter(post_intersection));

    std::cout << "Matches (" << post_intersection.size() << "):\n\n";

    std::vector<Post> firstOnLobsters;
    std::vector<Post> lastOnLobsters;
    std::vector<Post> firstOnHN;
    std::vector<Post> lastOnHN;
    std::vector<time_t> timeDiff;
    std::vector<int> lobstersScore;
    std::vector<int> lobstersComments;

    std::vector<int> hnScore;
    std::vector<int> hnComments;


    for (const auto &p : post_intersection)
    {
        auto hnPost = std::find_if(hnPosts.begin(), hnPosts.end(), [&cp = p](const Post &p) -> bool { return cp == p; });
        if (hnPost == hnPosts.end())
            continue;

        auto lobstersPost = std::find_if(lobstersPosts.begin(), lobstersPosts.end(), [&cp = p](const Post &p) -> bool { return cp == p; });
        if (lobstersPost == lobstersPosts.end())
            continue;

        Post firstPost = *lobstersPost;
        Post secondPost = *hnPost;
        std::string firstName = "Lobsters";
        std::string secondName = "HackerNews";
        if (hnPost->submit_timestamp < lobstersPost->submit_timestamp)
        {
            firstOnHN.push_back(p);
            lastOnLobsters.push_back(p);
            std::swap(firstName, secondName);
            std::swap(firstPost, secondPost);
        }
        else
        {
            firstOnLobsters.push_back(p);
            lastOnHN.push_back(p);
        }

        lobstersComments.push_back(lobstersPost->comment_count);
        lobstersScore.push_back(lobstersPost->votes);
        hnComments.push_back(hnPost->comment_count);
        hnScore.push_back(hnPost->votes);

        std::cout << "# " << p.title << "  \nURL: " << p.original_url << "  \n";

        std::cout << "First appeared on **" << firstName << "** with " << firstPost.votes
                  << " votes and " << firstPost.comment_count << " comments, submitted by "
                  << firstPost.submitter << " (" << firstPost.printDateTimeLocal() << "; "
                  << firstPost.comment_url << " ).  \n";

        time_t diffSec = difftime(secondPost.submit_timestamp, firstPost.submit_timestamp);

        timeDiff.push_back(diffSec);

        if (std::chrono::seconds(diffSec) < std::chrono::hours(1))
            std::cout << "**Within the hour this was also posted to " << secondName << "!**\n";

        tm *tp = gmtime(&diffSec); // utc
        std::cout << "After ";
        printTm(tp);

        std::cout << "it was submitted to **" << secondName << "** by " << secondPost.submitter << " with "
                  << secondPost.votes << " votes and " << secondPost.comment_count << " comments ("
                  << secondPost.printDateTimeLocal() << "; " << secondPost.comment_url << " ).  \n";

        std::string highestScore = (firstPost.votes > secondPost.votes) ? firstName : secondName;
        if ((firstPost.votes + secondPost.votes) <= 0)
            highestScore = "nowhere";

        std::string mostComments = (firstPost.comment_count > secondPost.comment_count) ? firstName : secondName;
        if ((firstPost.comment_count + secondPost.comment_count) <= 0)
            mostComments = "nowhere";

        std::cout << "The highest score was reached on " << highestScore
                  << " and the most comments were on " << mostComments << ".  \n";

        if (firstPost.submitter == secondPost.submitter)
            std::cout << "**The same username submitted the post to both sites**.  \n";

        std::cout << "\n";
    }

    std::cout << firstOnLobsters.size() << " posts appeared first on Lobsters and " << firstOnHN.size() << " posts appeared first on HackerNews.\n";

    time_t sum = std::accumulate(timeDiff.cbegin(), timeDiff.cend(), 0ll);
    time_t avg = sum / timeDiff.size();

    tm *diff_tp = gmtime(&avg);
    std::cout << "Average time for a cross-post: ";
    printTm(diff_tp);
    std::cout << ".\n";

    std::cout << "Average comments on HN: " << calcAverage(hnComments) << ", Lobsters: " << calcAverage(lobstersComments) << ".\n";
    std::cout << "Average score on HN: " << calcAverage(hnScore) << ", Lobsters: " << calcAverage(lobstersScore) << ".\n";

}


std::vector<std::string>& Arguments()
{
    static std::vector<std::string> arguments;
    return arguments;
}

void usage() {
    std::cout << "Usage: " << Arguments().at(0) << " [help|test|top|new]\n";
    std::cout << Arguments().at(0) << " top: analyze top stories from HN & Lobsters.\n";
    std::cout << Arguments().at(0) << " help: this text.\n";
    std::cout << Arguments().at(0) << " test: run a test to check your timezones.\n";
    std::cout << Arguments().at(0) << " new: get new posts instead of best.\n";
}

int main(int argc, char* argv[])
{
    for (int i = 0; i < argc; ++i)
    {
        Arguments().push_back(argv[i]);
    }


#ifndef __GNUG__
    std::cout << "Please use GCC to compile, we're using it's struct tm tm_gmtoff extension.";
    return 1;
#endif

    std::cout << "Which stories appear both on Lobsters and on HN, who was first?\n";
    std::cout << "An excuse to play with parsing a JSON api in C++ with async by Remy van Elst (https://raymii.org)\n\n";

    printCurrentDate();

    auto lobster = lobsters("lobste.rs", "/page/%PAGENUMBER%.json");
    auto hn = hackernews("hacker-news.firebaseio.com", "/v0/beststories.json", "/v0/item/%ID%.json");

    if (Arguments().size() >= 2 && Arguments().at(1) == "help") {
        usage();
        return 0;
    }

    if (Arguments().size() >= 2 && Arguments().at(1) == "new") {
        lobster = lobsters("lobste.rs", "/newest/page/%PAGENUMBER%.json");
        hn = hackernews("hacker-news.firebaseio.com", "/v0/newstories.json", "/v0/item/%ID%.json");

        std::cout << "Fetching HackerNews New Stories async (200 posts) (https://github.com/HackerNews/API)\n";
        std::vector<Post> hnPosts = hn.parsePosts(hn.getPosts());

        std::cout << "Fetching the first ten Lobsters pages (/newest) async 10*25=200 posts) (https://lobste.rs/s/r9oskz/is_there_api_documentation_for_lobsters_somewhere)\n\n";
        std::vector<Post> lobstersPosts = lobster.parsePosts(lobster.getPosts());

        analyze(lobstersPosts, hnPosts);
        return 0;
    }

    if (Arguments().size() >= 2 && Arguments().at(1) == "test")
    {
        /* hn time 1609074256                   converts to GMT:  Sunday December 27, 2020 13:04:16
       lobsters time 2020-12-27T06:58-06:00 converts to GMT:  Sunday December 27, 2020 12:58:40
       difference should be 5 minutes, 36 seconds.
    */
        std::cout << "--- START TEST ---\nDate/time/timezones are hard. Below is a test post comparison,"
                     "check if your timezone information is correct. The difference between Lobsters and "
                     "HN should be 5 minutes and 36 seconds. \n";
        std::string lobsters_test_json = "[[{\"short_id\":\"4pivy1\",\"short_id_url\":\"https://lobste.rs/s/4pivy1\",\"created_at\":\"2020-12-27T06:58:40.000-06:00\",\"title\":\"Bash HTTP monitoring dashboard\",\"url\":\"https://raymii.org/s/software/Bash_HTTP_Monitoring_Dashboard.html\",\"score\":30,\"flags\":0,\"comment_count\":2,\"description\":\"\",\"comments_url\":\"https://lobste.rs/s/4pivy1/bash_http_monitoring_dashboard\",\"submitter_user\":{\"username\":\"raymii\",\"created_at\":\"2013-11-20T11:58:43.000-06:00\",\"is_admin\":false,\"about\":\"https://raymii.org\",\"is_moderator\":false,\"karma\":7351,\"avatar_url\":\"/avatars/raymii-100.png\",\"invited_by_user\":\"journeysquid\"},\"tags\":[\"linux\",\"web\"],\"comments\":[{\"short_id\":\"zdonpb\",\"short_id_url\":\"https://lobste.rs/c/zdonpb\",\"created_at\":\"2020-12-28T06:50:10.000-06:00\",\"updated_at\":\"2020-12-28T06:51:33.000-06:00\",\"is_deleted\":false,\"is_moderated\":false,\"score\":2,\"flags\":0,\"comment\":\"\\u003cp\\u003eThanks Remy, I enjoyed reading through the shell script source, which inspired me to write a \\u003ca href=\\\"https://lobste.rs/s/2ougg7/waiting_for_jobs_concept_shell\\\" rel=\\\"ugc\\\"\\u003epost about \\u003ccode\\u003ewait\\u003c/code\\u003e, and about shell scripting\\u003c/a\\u003e today.\\u003c/p\\u003e\\n\",\"url\":\"https://lobste.rs/s/4pivy1/bash_http_monitoring_dashboard#c_zdonpb\",\"indent_level\":1,\"commenting_user\":{\"username\":\"qmacro\",\"created_at\":\"2020-01-24T10:48:42.000-06:00\",\"is_admin\":false,\"about\":\"[Developer, author, teacher, speaker](https://qmacro.org). And fascinated by all sorts of stuff.\",\"is_moderator\":false,\"karma\":79,\"avatar_url\":\"/avatars/qmacro-100.png\",\"invited_by_user\":\"martinrue\",\"github_username\":\"qmacro\",\"twitter_username\":\"qmacro\"}},{\"short_id\":\"lalafr\",\"short_id_url\":\"https://lobste.rs/c/lalafr\",\"created_at\":\"2020-12-28T08:38:37.000-06:00\",\"updated_at\":\"2020-12-28T08:38:37.000-06:00\",\"is_deleted\":false,\"is_moderated\":false,\"score\":3,\"flags\":0,\"comment\":\"\\u003cp\\u003eThat is a great post, fun to read. I like such posts with backstory and musings. Often unable to write those myself, I’d rather stick to guides.\\u003c/p\\u003e\\n\\u003cp\\u003eSubscribed to your rss feed as well.\",\"url\":\"https://lobste.rs/s/4pivy1/bash_http_monitoring_dashboard#c_lalafr\",\"indent_level\":2,\"commenting_user\":{\"username\":\"raymii\",\"created_at\":\"2013-11-20T11:58:43.000-06:00\",\"is_admin\":false,\"about\":\"https://raymii.org\",\"is_moderator\":false,\"karma\":7351,\"avatar_url\":\"/avatars/raymii-100.png\",\"invited_by_user\":\"journeysquid\"}}]}]]";
        std::string hn_test_json = "[{\"by\":\"todsacerdoti\",\"descendants\":26,\"id\":25550732,\"kids\":[25551346,25551828,25552963,25556255,25552339,25559309,25554106,25553520,25552809,25557037],\"score\":154,\"time\":1609074256,\"title\":\"Bash HTTP Monitoring Dashboard\",\"type\":\"story\",\"url\":\"https://raymii.org/s/software/Bash_HTTP_Monitoring_Dashboard.html\"}]";

        std::vector<Post> test_hnPosts = hn.parsePosts(json::parse(hn_test_json));
        std::vector<Post> test_lobstersPosts = lobster.parsePosts(json::parse(lobsters_test_json));
        analyze(test_lobstersPosts, test_hnPosts);

        std::cout << "--- END TEST ---\n\n";
        return 0;
    }

    if (Arguments().size() >= 2 && Arguments().at(1) == "top")
    {
        std::cout << "Fetching HackerNews Best Stories async (200 posts) (https://github.com/HackerNews/API)\n";
        std::vector<Post> hnPosts = hn.parsePosts(hn.getPosts());

        std::cout << "Fetching the first ten Lobsters pages async 10*25=200 posts) (https://lobste.rs/s/r9oskz/is_there_api_documentation_for_lobsters_somewhere)\n\n";
        std::vector<Post> lobstersPosts = lobster.parsePosts(lobster.getPosts());

        analyze(lobstersPosts, hnPosts);
        return 0;
    }

    usage();
    return 0;
}